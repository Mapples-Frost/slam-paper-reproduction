# room_dark_01 adaptive k summary

## Change
Modified `surfOptimization()` in `mapOptmization.cpp`:
- replaced the fixed residual coefficient `0.9`
- added `adaptiveResidualK(dis_point)`
- only changed point-to-plane residual weighting
- did not modify `cornerOptimization()`

## Evaluation setup
- dataset: room_dark_01
- imu topic: /handsfree/imu
- GT: `eval/formal/room_dark_01/gt/room_dark_01_gt_tum.txt`
- alignment: `--align`
- max time diff: `--t_max_diff 0.05`

## Baseline
- estimated poses: 555
- matched pairs: 547
- mean: 0.136145 m
- rmse: 0.148386 m
- max: 0.284917 m

## Adaptive k
- estimated poses: 555
- matched pairs: 547
- mean: 0.134612 m
- rmse: 0.146603 m
- max: 0.281849 m

## Comparison
Compared with the baseline on `room_dark_01`, the adaptive `k` version shows a small but consistent improvement:
- matched pairs unchanged: 547 -> 547
- mean APE improved from 0.136145 m to 0.134612 m (`-0.001533 m`, about `-1.13%`)
- rmse improved from 0.148386 m to 0.146603 m (`-0.001783 m`, about `-1.20%`)
- max improved from 0.284917 m to 0.281849 m (`-0.003068 m`, about `-1.08%`)

## Conclusion
On `room_dark_01`, introducing the adaptive residual coefficient `k` in `surfOptimization()` produced a modest but stable accuracy gain over the baseline while keeping the trajectory length and matched pose count unchanged. This indicates that the adaptive weighting strategy is beneficial for the LIO baseline on this sequence, although the improvement is relatively small and should be validated on more sequences later.
