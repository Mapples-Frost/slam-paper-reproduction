# Baseline Evaluation - walking_dataset

## Basic Info
- Dataset: walking_dataset.bag
- Dataset path: /home/mapples/slam_repro/datasets/lio_sam_samples/walking_dataset.bag
- Launch command: roslaunch lio_sam run.launch
- Playback command: rosbag play --clock -r 0.5 /home/mapples/slam_repro/datasets/lio_sam_samples/walking_dataset.bag

## Saved Outputs
- Topic bag: logs/baseline_output_topics.bag
- Topic bag info: logs/baseline_output_topics_info.txt
- Topic list: logs/topic_list.txt
- LIO odom topic info: logs/lio_odom_topic_info.txt
- Bag info: logs/bag_info.txt
- Playback log: logs/playback_time.txt
- Screenshot: screenshots/map_top_view.png

## Candidate Reference Trajectory
- Topic: /gx5/nav/odom
- Expected type: nav_msgs/Odometry
- Note: recorded as candidate reference trajectory; needs validation before being treated as GT-equivalent reference

## Qualitative Observations
- 是否完整跑完：yes
- 轨迹是否连续：yes
- 地图是否正常成图：yes
- 是否出现黑屏：无黑屏
- 备注：本轮采用较稳的播放方式进行基线留档

## Next Step
- Verify whether /gx5/nav/odom was successfully recorded in baseline_output_topics.bag
- Prepare trajectory comparison for later APE evaluation
- Keep this run as baseline for later modified versions
