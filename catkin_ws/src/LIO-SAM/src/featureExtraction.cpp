#include "utility.h"
#include "lio_sam/cloud_info.h"
#include <unordered_map>
#include <unordered_set>
#include <queue>
#include <cfloat>

struct smoothness_t{
    float value;
    size_t ind;
};

struct by_value{
    bool operator()(smoothness_t const &left, smoothness_t const &right) {
        return left.value < right.value;
    }
};

struct BevCellStat
{
    int count = 0;
    float sumZ = 0.0f;
    float minZ = FLT_MAX;
    float maxZ = -FLT_MAX;
    float sumRange = 0.0f;
    std::vector<int> indices;
};

class FeatureExtraction : public ParamServer
{
public:
    ros::Subscriber subLaserCloudInfo;
    ros::Publisher pubLaserCloudInfo;
    ros::Publisher pubCornerPoints;
    ros::Publisher pubSurfacePoints;

    pcl::PointCloud<PointType>::Ptr extractedCloud;
    pcl::PointCloud<PointType>::Ptr cornerCloud;
    pcl::PointCloud<PointType>::Ptr surfaceCloud;

    pcl::VoxelGrid<PointType> downSizeFilter;

    lio_sam::cloud_info cloudInfo;
    std_msgs::Header cloudHeader;

    std::vector<smoothness_t> cloudSmoothness;

    float *cloudCurvature;
    int *cloudNeighborPicked;
    int *cloudLabel;
    std::vector<int> cloudDynamicMask;

    FeatureExtraction()
    {
        subLaserCloudInfo = nh.subscribe<lio_sam::cloud_info>(
            "lio_sam/deskew/cloud_info", 1,
            &FeatureExtraction::laserCloudInfoHandler, this,
            ros::TransportHints().tcpNoDelay());

        pubLaserCloudInfo = nh.advertise<lio_sam::cloud_info>("lio_sam/feature/cloud_info", 1);
        pubCornerPoints   = nh.advertise<sensor_msgs::PointCloud2>("lio_sam/feature/cloud_corner", 1);
        pubSurfacePoints  = nh.advertise<sensor_msgs::PointCloud2>("lio_sam/feature/cloud_surface", 1);

        initializationValue();
    }

    void initializationValue()
    {
        cloudSmoothness.resize(N_SCAN * Horizon_SCAN);
        downSizeFilter.setLeafSize(odometrySurfLeafSize, odometrySurfLeafSize, odometrySurfLeafSize);

        extractedCloud.reset(new pcl::PointCloud<PointType>());
        cornerCloud.reset(new pcl::PointCloud<PointType>());
        surfaceCloud.reset(new pcl::PointCloud<PointType>());

        cloudCurvature = new float[N_SCAN * Horizon_SCAN];
        cloudNeighborPicked = new int[N_SCAN * Horizon_SCAN];
        cloudLabel = new int[N_SCAN * Horizon_SCAN];
        cloudDynamicMask.resize(N_SCAN * Horizon_SCAN, 0);
    }

    void laserCloudInfoHandler(const lio_sam::cloud_infoConstPtr& msgIn)
    {
        cloudInfo = *msgIn;
        cloudHeader = msgIn->header;
        pcl::fromROSMsg(msgIn->cloud_deskewed, *extractedCloud);

        calculateSmoothness();
        detectDynamicPoints();
        markOccludedPoints();
        extractFeatures();
        publishFeatureCloud();
    }

private:
    inline Eigen::Vector3f pointToVec(int idx) const
    {
        const auto &pt = extractedCloud->points[idx];
        return Eigen::Vector3f(pt.x, pt.y, pt.z);
    }

    inline bool validPointIndex(int idx) const
    {
        return idx >= 0 && idx < static_cast<int>(extractedCloud->points.size());
    }

    inline float originalRangeCurvature(int i) const
    {
        float diffRange = cloudInfo.pointRange[i-5] + cloudInfo.pointRange[i-4]
                        + cloudInfo.pointRange[i-3] + cloudInfo.pointRange[i-2]
                        + cloudInfo.pointRange[i-1] - cloudInfo.pointRange[i] * 10.0f
                        + cloudInfo.pointRange[i+1] + cloudInfo.pointRange[i+2]
                        + cloudInfo.pointRange[i+3] + cloudInfo.pointRange[i+4]
                        + cloudInfo.pointRange[i+5];
        return diffRange * diffRange;
    }

    void resetFeatureArrays(int cloudSize)
    {
        if (static_cast<int>(cloudDynamicMask.size()) < cloudSize)
            cloudDynamicMask.resize(cloudSize, 0);

        for (int i = 0; i < cloudSize; ++i)
        {
            cloudCurvature[i] = 0.0f;
            cloudNeighborPicked[i] = 0;
            cloudLabel[i] = 0;
            cloudDynamicMask[i] = 0;
            cloudSmoothness[i].value = 0.0f;
            cloudSmoothness[i].ind = i;
        }
    }

    void buildRingColumnIndex(std::vector<std::vector<int>> &ringColToIndex,
                              std::vector<int> &indexToRing) const
    {
        const int cloudSize = extractedCloud->points.size();
        ringColToIndex.assign(N_SCAN, std::vector<int>(Horizon_SCAN, -1));
        indexToRing.assign(cloudSize, -1);

        for (int ring = 0; ring < N_SCAN; ++ring)
        {
            if (ring >= static_cast<int>(cloudInfo.startRingIndex.size()) ||
                ring >= static_cast<int>(cloudInfo.endRingIndex.size()))
                continue;

            int startIdx = std::max(0, cloudInfo.startRingIndex[ring]);
            int endIdx   = std::min(cloudSize - 1, cloudInfo.endRingIndex[ring]);

            for (int i = startIdx; i <= endIdx; ++i)
            {
                int col = cloudInfo.pointColInd[i];
                if (col >= 0 && col < Horizon_SCAN)
                {
                    ringColToIndex[ring][col] = i;
                    indexToRing[i] = ring;
                }
            }
        }
    }

    bool addNeighborIfValid(std::vector<int> &neighbors,
                            const std::vector<std::vector<int>> &ringColToIndex,
                            int ring, int col, float centerRange) const
    {
        if (ring < 0 || ring >= N_SCAN || col < 0 || col >= Horizon_SCAN)
            return false;

        int idx = ringColToIndex[ring][col];
        if (!validPointIndex(idx))
            return false;

        float neighborRange = cloudInfo.pointRange[idx];
        float gate = std::max(0.5f, 0.10f * centerRange);
        if (std::fabs(neighborRange - centerRange) > gate)
            return false;

        neighbors.push_back(idx);
        return true;
    }

    float hybridSurfaceCurvature(int i,
                                 const std::vector<std::vector<int>> &ringColToIndex,
                                 const std::vector<int> &indexToRing) const
    {
        if (i < 5 || i >= static_cast<int>(extractedCloud->points.size()) - 5)
            return 0.0f;

        int ring = indexToRing[i];
        if (ring < 0)
            return 0.0f;

        int col = cloudInfo.pointColInd[i];
        if (col < 0 || col >= Horizon_SCAN)
            return 0.0f;

        float rangeCurv = originalRangeCurvature(i);
        float centerRange = std::max(cloudInfo.pointRange[i], 1e-3f);

        std::vector<int> neighbors;
        neighbors.reserve(16);

        int sameRingCount = 0;
        int adjacentCount = 0;

        for (int dc = -3; dc <= 3; ++dc)
        {
            if (dc == 0)
                continue;
            if (addNeighborIfValid(neighbors, ringColToIndex, ring, col + dc, centerRange))
                ++sameRingCount;
        }

        for (int dc = -2; dc <= 2; ++dc)
        {
            if (addNeighborIfValid(neighbors, ringColToIndex, ring - 1, col + dc, centerRange))
                ++adjacentCount;
            if (addNeighborIfValid(neighbors, ringColToIndex, ring + 1, col + dc, centerRange))
                ++adjacentCount;
        }

        if (sameRingCount < 4 || adjacentCount < 4)
            return rangeCurv;

        Eigen::Vector3f center = pointToVec(i);
        float centerNorm = std::max(center.norm(), 1e-3f);
        Eigen::Vector3f accum = Eigen::Vector3f::Zero();

        for (int idx : neighbors)
            accum += (pointToVec(idx) - center);

        float surfaceTerm = accum.norm() / (std::max(1.0f, static_cast<float>(sameRingCount)) * centerNorm);

        if (!std::isfinite(surfaceTerm))
            return rangeCurv;

        surfaceTerm = std::min(surfaceTerm, 1.0f);

        constexpr float fusionAlpha = 0.25f;
        return rangeCurv * (1.0f + fusionAlpha * surfaceTerm);
    }

    void markNeighborhoodPicked(int ind)
    {
        if (!validPointIndex(ind))
            return;

        cloudNeighborPicked[ind] = 1;

        for (int l = 1; l <= 5; ++l)
        {
            if (!validPointIndex(ind + l) || !validPointIndex(ind + l - 1))
                break;
            int columnDiff = std::abs(int(cloudInfo.pointColInd[ind + l] - cloudInfo.pointColInd[ind + l - 1]));
            if (columnDiff > 10)
                break;
            cloudNeighborPicked[ind + l] = 1;
        }

        for (int l = -1; l >= -5; --l)
        {
            if (!validPointIndex(ind + l) || !validPointIndex(ind + l + 1))
                break;
            int columnDiff = std::abs(int(cloudInfo.pointColInd[ind + l] - cloudInfo.pointColInd[ind + l + 1]));
            if (columnDiff > 10)
                break;
            cloudNeighborPicked[ind + l] = 1;
        }
    }

    void detectDynamicPoints()
    {
        const float cellSize = 0.35f;
        const float rangeMax = 12.0f;
        const float minRange = 0.8f;

        std::unordered_map<long long, BevCellStat> grid;
        grid.reserve(extractedCloud->points.size() / 4 + 1);

        auto cellKey = [](int gx, int gy) -> long long {
            return (static_cast<long long>(gx) << 32) ^ (static_cast<unsigned int>(gy));
        };

        auto unpack = [](long long key) -> std::pair<int,int> {
            int gx = static_cast<int>(key >> 32);
            int gy = static_cast<int>(key & 0xffffffff);
            return {gx, gy};
        };

        for (int i = 0; i < static_cast<int>(extractedCloud->points.size()); ++i)
        {
            const auto &pt = extractedCloud->points[i];
            float r = cloudInfo.pointRange[i];
            if (!std::isfinite(pt.x) || !std::isfinite(pt.y) || !std::isfinite(pt.z))
                continue;
            if (r < minRange || r > rangeMax)
                continue;

            int gx = static_cast<int>(std::floor(pt.x / cellSize));
            int gy = static_cast<int>(std::floor(pt.y / cellSize));
            long long key = cellKey(gx, gy);

            auto &cell = grid[key];
            cell.count += 1;
            cell.sumZ += pt.z;
            cell.minZ = std::min(cell.minZ, pt.z);
            cell.maxZ = std::max(cell.maxZ, pt.z);
            cell.sumRange += r;
            cell.indices.push_back(i);
        }

        std::unordered_set<long long> candidateCells;
        candidateCells.reserve(grid.size());

        for (const auto &kv : grid)
        {
            const auto &cell = kv.second;
            if (cell.count < 4 || cell.count > 120)
                continue;

            float meanZ = cell.sumZ / static_cast<float>(cell.count);
            float meanRange = cell.sumRange / static_cast<float>(cell.count);
            float heightSpan = cell.maxZ - cell.minZ;

            // heuristic: compact elevated clutter / human-like vertical structure
            if (meanRange < 10.0f &&
                meanZ > -0.8f &&
                cell.maxZ > 0.15f &&
                heightSpan > 0.55f &&
                heightSpan < 2.4f)
            {
                candidateCells.insert(kv.first);
            }
        }

        std::unordered_set<long long> visited;
        visited.reserve(candidateCells.size());

        const int dx[8] = {1,1,0,-1,-1,-1,0,1};
        const int dy[8] = {0,1,1,1,0,-1,-1,-1};

        for (long long startKey : candidateCells)
        {
            if (visited.count(startKey))
                continue;

            std::queue<long long> q;
            std::vector<long long> component;
            q.push(startKey);
            visited.insert(startKey);

            int minGx = INT_MAX, minGy = INT_MAX, maxGx = INT_MIN, maxGy = INT_MIN;
            int totalPoints = 0;
            float compMinZ = FLT_MAX, compMaxZ = -FLT_MAX;

            while (!q.empty())
            {
                long long cur = q.front();
                q.pop();
                component.push_back(cur);

                auto [gx, gy] = unpack(cur);
                minGx = std::min(minGx, gx);
                minGy = std::min(minGy, gy);
                maxGx = std::max(maxGx, gx);
                maxGy = std::max(maxGy, gy);

                const auto &cell = grid[cur];
                totalPoints += cell.count;
                compMinZ = std::min(compMinZ, cell.minZ);
                compMaxZ = std::max(compMaxZ, cell.maxZ);

                for (int k = 0; k < 8; ++k)
                {
                    long long nxt = cellKey(gx + dx[k], gy + dy[k]);
                    if (!candidateCells.count(nxt) || visited.count(nxt))
                        continue;
                    visited.insert(nxt);
                    q.push(nxt);
                }
            }

            int widthCells = maxGx - minGx + 1;
            int heightCells = maxGy - minGy + 1;
            int areaCells = widthCells * heightCells;
            float compHeight = compMaxZ - compMinZ;

            bool dynamicCluster =
                static_cast<int>(component.size()) >= 1 &&
                static_cast<int>(component.size()) <= 20 &&
                areaCells <= 28 &&
                totalPoints >= 8 &&
                totalPoints <= 600 &&
                compHeight > 0.55f &&
                compHeight < 2.6f;

            if (!dynamicCluster)
                continue;

            for (long long key : component)
            {
                for (int idx : grid[key].indices)
                    cloudDynamicMask[idx] = 1;
            }
        }

        for (int i = 0; i < static_cast<int>(extractedCloud->points.size()); ++i)
        {
            if (cloudDynamicMask[i])
                cloudNeighborPicked[i] = 1;
        }
    }

public:
    void calculateSmoothness()
    {
        const int cloudSize = extractedCloud->points.size();
        resetFeatureArrays(cloudSize);

        std::vector<std::vector<int>> ringColToIndex;
        std::vector<int> indexToRing;
        buildRingColumnIndex(ringColToIndex, indexToRing);

        for (int i = 5; i < cloudSize - 5; ++i)
        {
            cloudCurvature[i] = hybridSurfaceCurvature(i, ringColToIndex, indexToRing);
            cloudSmoothness[i].value = cloudCurvature[i];
            cloudSmoothness[i].ind = i;
        }
    }

    void markOccludedPoints()
    {
        int cloudSize = extractedCloud->points.size();

        for (int i = 5; i < cloudSize - 6; ++i)
        {
            if (cloudDynamicMask[i])
            {
                cloudNeighborPicked[i] = 1;
                continue;
            }

            float depth1 = cloudInfo.pointRange[i];
            float depth2 = cloudInfo.pointRange[i + 1];
            int columnDiff = std::abs(int(cloudInfo.pointColInd[i + 1] - cloudInfo.pointColInd[i]));

            if (columnDiff < 10)
            {
                if (depth1 - depth2 > 0.3f)
                {
                    for (int k = -5; k <= 0; ++k)
                    {
                        if (validPointIndex(i + k))
                            cloudNeighborPicked[i + k] = 1;
                    }
                }
                else if (depth2 - depth1 > 0.3f)
                {
                    for (int k = 1; k <= 6; ++k)
                    {
                        if (validPointIndex(i + k))
                            cloudNeighborPicked[i + k] = 1;
                    }
                }
            }

            float diff1 = std::abs(float(cloudInfo.pointRange[i - 1] - cloudInfo.pointRange[i]));
            float diff2 = std::abs(float(cloudInfo.pointRange[i + 1] - cloudInfo.pointRange[i]));

            if (diff1 > 0.02f * cloudInfo.pointRange[i] && diff2 > 0.02f * cloudInfo.pointRange[i])
                cloudNeighborPicked[i] = 1;
        }
    }

    void extractFeatures()
    {
        cornerCloud->clear();
        surfaceCloud->clear();

        pcl::PointCloud<PointType>::Ptr surfaceCloudScan(new pcl::PointCloud<PointType>());
        pcl::PointCloud<PointType>::Ptr surfaceCloudScanDS(new pcl::PointCloud<PointType>());

        for (int i = 0; i < N_SCAN; ++i)
        {
            surfaceCloudScan->clear();

            for (int j = 0; j < 6; ++j)
            {
                int sp = (cloudInfo.startRingIndex[i] * (6 - j) + cloudInfo.endRingIndex[i] * j) / 6;
                int ep = (cloudInfo.startRingIndex[i] * (5 - j) + cloudInfo.endRingIndex[i] * (j + 1)) / 6 - 1;

                if (sp >= ep)
                    continue;

                std::sort(cloudSmoothness.begin() + sp, cloudSmoothness.begin() + ep, by_value());

                int largestPickedNum = 0;
                for (int k = ep; k >= sp; --k)
                {
                    int ind = cloudSmoothness[k].ind;

                    if (cloudDynamicMask[ind])
                        continue;

                    if (cloudNeighborPicked[ind] == 0 && cloudCurvature[ind] > edgeThreshold)
                    {
                        ++largestPickedNum;
                        if (largestPickedNum <= 20)
                        {
                            cloudLabel[ind] = 1;
                            cornerCloud->push_back(extractedCloud->points[ind]);
                        }
                        else
                        {
                            break;
                        }

                        markNeighborhoodPicked(ind);
                    }
                }

                for (int k = sp; k <= ep; ++k)
                {
                    int ind = cloudSmoothness[k].ind;

                    if (cloudDynamicMask[ind])
                        continue;

                    if (cloudNeighborPicked[ind] == 0 && cloudCurvature[ind] < surfThreshold)
                    {
                        cloudLabel[ind] = -1;
                        markNeighborhoodPicked(ind);
                    }
                }

                for (int k = sp; k <= ep; ++k)
                {
                    int ind = cloudSmoothness[k].ind;
                    if (!cloudDynamicMask[ind] && cloudLabel[ind] <= 0)
                        surfaceCloudScan->push_back(extractedCloud->points[ind]);
                }
            }

            surfaceCloudScanDS->clear();
            downSizeFilter.setInputCloud(surfaceCloudScan);
            downSizeFilter.filter(*surfaceCloudScanDS);

            *surfaceCloud += *surfaceCloudScanDS;
        }
    }

    void freeCloudInfoMemory()
    {
        cloudInfo.startRingIndex.clear();
        cloudInfo.endRingIndex.clear();
        cloudInfo.pointColInd.clear();
        cloudInfo.pointRange.clear();
    }

    void publishFeatureCloud()
    {
        freeCloudInfoMemory();

        cloudInfo.cloud_corner = publishCloud(pubCornerPoints, cornerCloud, cloudHeader.stamp, lidarFrame);
        cloudInfo.cloud_surface = publishCloud(pubSurfacePoints, surfaceCloud, cloudHeader.stamp, lidarFrame);

        pubLaserCloudInfo.publish(cloudInfo);
    }
};

int main(int argc, char** argv)
{
    ros::init(argc, argv, "lio_sam");

    FeatureExtraction FE;
    ROS_INFO("\033[1;32m----> Feature Extraction Started.\033[0m");

    ros::spin();

    return 0;
}
