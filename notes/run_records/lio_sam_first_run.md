# LIO-SAM 第一次成功运行记录

## 日期
今天

## 使用的数据
walking_dataset.bag

## 数据位置
~/slam_repro/datasets/lio_sam_samples/walking_dataset.bag

## 启动命令
终端1：
source /opt/ros/noetic/setup.bash
source ~/slam_repro/catkin_ws/devel/setup.bash
roslaunch lio_sam run.launch

终端2：
source /opt/ros/noetic/setup.bash
source ~/slam_repro/catkin_ws/devel/setup.bash
cd ~/slam_repro/datasets/lio_sam_samples
rosbag play walking_dataset.bag -r 3

## 运行结果
1. RViz 正常打开
2. 点云正常显示
3. 轨迹正常显示
4. 地图开始累积
5. LIO-SAM 能够处理官方示例数据

## 备注
这是第一套成功跑通的现成系统，后面所有论文复现都以这个状态为基线。
