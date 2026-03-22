# Stage 5 - featureExtraction.cpp 定位笔记

## 当前定位目标

分析 LIO-SAM 中特征提取模块的 baseline 实现，明确：
1. 当前曲率如何定义
2. 当前遮挡点如何过滤
3. 当前角点 / 面点如何筛选
4. 论文下一步最可能修改的代码位置

---

## 目标源码文件

- catkin_ws/src/LIO-SAM/src/featureExtraction.cpp

---

## 关键函数定位

- calculateSmoothness()
- markOccludedPoints()
- extractFeatures()

grep 结果：
- calculateSmoothness(): line 81
- markOccludedPoints(): line 103
- extractFeatures(): line 141
- edgeThreshold usage: line 168
- surfThreshold usage: line 199
- largestPickedNum usage: lines 164, 170, 171

---

## baseline 逻辑总结

### 1. calculateSmoothness()

作用：
- 计算每个点的曲率 / 平滑度响应

当前实现：
- 对点 i，取前后各 5 个点的 range
- 使用：
  previous 5 ranges + next 5 ranges - current range * 10
- 将差分结果平方：
  cloudCurvature[i] = diffRange * diffRange

说明：
- 当前 baseline 的曲率定义基于 range image 的距离差分
- 不是直接基于三维坐标差分
- 如果论文修改“曲率定义”，这里是第一优先修改点

---

### 2. markOccludedPoints()

作用：
- 过滤遮挡点和不稳定点，避免后续被选成特征点

当前实现包含两类规则：

1) occluded points
- 比较相邻列、相邻深度
- 如果列差 < 10 且深度跳变 > 0.3
- 则将一段邻域点标记为 cloudNeighborPicked = 1

2) parallel beam points
- 若当前点与左右邻点的 range 差都超过 0.02 * 当前 range
- 则将当前点标为 cloudNeighborPicked = 1

说明：
- 该函数主要用于预先剔除不可靠点
- 如果论文没有明确改遮挡处理，这里不是当前第一优先改动点

---

### 3. extractFeatures()

作用：
- 从当前帧中提取角点和面点特征

当前实现流程：

1) 按扫描线处理
- for each ring in N_SCAN

2) 每条扫描线切分为 6 段
- 保证特征分布均匀

3) 对每段按曲率排序
- 使用 cloudSmoothness 排序

4) 提取角点
- 条件：cloudNeighborPicked[ind] == 0
- 条件：cloudCurvature[ind] > edgeThreshold
- 每段最多取 20 个角点
- 选中后将邻近 ±5 个点标记，避免特征过密

5) 提取面点
- 条件：cloudNeighborPicked[ind] == 0
- 条件：cloudCurvature[ind] < surfThreshold
- 标记为 cloudLabel[ind] = -1
- 同样标记邻近点，避免过密

6) 面点集合下采样
- 使用 downSizeFilter 对 surfaceCloudScan 做体素下采样
- 再并入最终 surfaceCloud

说明：
- extractFeatures() 是当前角点 / 面点筛选的主逻辑
- 如果论文修改“特征筛选规则 / 阈值 / 选点策略 / 分段策略”，这里是第一优先修改点

---

## 当前阶段结论

已经确认：

1. baseline 曲率定义在 calculateSmoothness()
2. 角点 / 面点筛选主逻辑在 extractFeatures()
3. markOccludedPoints() 主要负责辅助过滤
4. 论文下一步最可能的改动位置：
   - 曲率定义类改动 -> calculateSmoothness()
   - 特征筛选类改动 -> extractFeatures()

---

## 下一步计划

下一步不直接改代码，先回到论文文本，确认“曲率 / 特征提取改法”更接近以下哪一类：

- 修改曲率定义
- 修改 edge / surf 特征筛选规则
- 修改分段策略
- 修改遮挡 / 动态点过滤策略

确认后再做最小改动复现，并继续在 room_dark_01 上用同一套 APE 流程做前后对比。
