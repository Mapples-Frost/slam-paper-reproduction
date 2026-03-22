# room_dark_01 feature refactor summary

## Change

This experiment was conducted on top of the previously added adaptive residual coefficient `k` in `mapOptmization.cpp`.

The current version therefore represents:

- baseline LIO-SAM
- + adaptive `k`
- + feature extraction refactor in `featureExtraction.cpp`

The feature extraction side was refactored with a hybrid surface-curvature pipeline.

---

## Evaluation setup

- dataset: `room_dark_01`
- GT: `eval/formal/room_dark_01/gt/room_dark_01_gt_tum.txt`
- alignment: `--align`
- max time diff: `--t_max_diff 0.05`

---

## Baseline

- estimated poses: 555
- matched pairs: 547
- mean: 0.136145 m
- rmse: 0.148386 m
- max: 0.284917 m

---

## Adaptive k only

- estimated poses: 555
- matched pairs: 547
- mean: 0.134612 m
- rmse: 0.146603 m
- max: 0.281849 m

---

## Adaptive k + feature refactor

- estimated poses: 555
- matched pairs: 547
- mean: 0.134606 m
- rmse: 0.146834 m
- max: 0.288260 m

---

## Comparison

Compared with the baseline:

- mean improved from 0.136145 m to 0.134606 m
- rmse improved from 0.148386 m to 0.146834 m
- max worsened from 0.284917 m to 0.288260 m

Compared with adaptive `k` only:

- mean is almost unchanged: 0.134612 m -> 0.134606 m
- rmse is slightly worse: 0.146603 m -> 0.146834 m
- max is worse: 0.281849 m -> 0.288260 m

---

## Conclusion

The current `adaptive k + feature refactor` version remains stable and keeps the same matched-pair count as previous runs, so the evaluation is comparable.

However, on `room_dark_01`, the feature refactor does not provide a clear additional gain over the `adaptive k` result:

- mean is nearly identical
- rmse is slightly worse
- max error is worse

Therefore, at the current implementation stage, the feature refactor is not yet a clearly beneficial addition on top of adaptive `k` for this sequence.
