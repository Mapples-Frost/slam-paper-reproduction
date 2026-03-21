# 论文约束复现说明（阶段1）

## 系统主干
LiDAR preprocessing -> dynamic object detection -> LIO -> VIO -> multi-sensor fusion optimization

## 论文核心改进
1. 动态点云过滤：FCN + BEV + 2D grid features
2. VIO 滑窗优化：两步式 marginalization / Schur complement
3. LIO 改进：
   - 跨扫描线曲率
   - 自适应点到面残差权重

## 论文实验目标
1. EuRoC: MH_01, MH_02, MH_03
2. KITTI bag: kitti_2011_09_26_drive_0009_synced.bag
3. M2DGR: room_01, door_01, dark_room_01, street_04
4. 评估工具：evo
5. 指标：APE

## 当前策略
1. 先不碰真机
2. 先搭环境和实验骨架
3. 先跑通基线，再插入论文改进
4. 每一步都区分：
   - 论文直接支持
   - 工程补全实现
