# LIO-SAM 代码结构初步理解

## 启动文件
run.launch
作用：
- 启动整个 LIO-SAM 系统
- 加载参数文件
- 启动各个节点

## 参数文件
params.yaml
作用：
- 配置点云、IMU、topic、雷达参数、外参等

## 核心源码文件
1. imageProjection.cpp
作用：
- 点云预处理
- 接收点云与 IMU
- 点云去畸变
- 点云整理

2. featureExtraction.cpp
作用：
- 提取边缘点和平面点

3. imuPreintegration.cpp
作用：
- 处理 IMU 信息
- 提供运动约束

4. mapOptmization.cpp
作用：
- 地图优化
- 当前帧与地图匹配
- 维护关键帧和地图
