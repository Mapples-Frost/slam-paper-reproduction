# M2DGR 正式评测计划

## 目标
将 walking_dataset 从“跑通样本”切换为 M2DGR 正式评测路线。

## 已确认结论
- walking_dataset 不能用于正式 APE 评测
- 本地已有 datasets/m2dgr 目录
- 当前尚未发现 room_01 / room_dark_01 / door_01 / street_04 的正式序列文件
- 当前在分支 stage4-baseline-eval 上工作

## 待准备序列
- room_01
- room_dark_01
- door_01
- street_04

## 后续流程
1. 下载各序列的 rosbag 和 GT
2. 确认 GT 文件格式
3. 运行 LIO-SAM 基线
4. 导出估计轨迹
5. 用 evo 做 APE
6. 固定 baseline 结果
7. 再开始修改论文相关部分（如自适应残差中的 k）
