# room_dark_01 baseline 结果

## 运行对象
- 数据集序列: room_dark_01
- 方法: LIO-SAM baseline
- IMU topic: /handsfree/imu
- LiDAR topic: /velodyne_points

## GT 轨迹
- 文件: eval/formal/room_dark_01/gt/room_dark_01_gt_tum.txt
- 位姿点数: 5621

## 估计轨迹
- 文件: eval/formal/room_dark_01/traj/room_dark_01_lio_baseline.tum
- 位姿点数: 555

## APE 结果
- 匹配位姿对数: 547
- max: 0.284917 m
- mean: 0.136145 m
- median: 0.131400 m
- min: 0.022567 m
- rmse: 0.148386 m
- std: 0.059015 m

## 说明
- 使用了 --align
- 使用了 --t_max_diff 0.05
- 该结果作为后续论文改动前的正式 baseline
