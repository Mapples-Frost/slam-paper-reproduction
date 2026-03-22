# room_dark_01 overall summary

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

Compared with baseline:

- mean improved from 0.136145 m to 0.134612 m
- rmse improved from 0.148386 m to 0.146603 m
- max improved from 0.284917 m to 0.281849 m

Conclusion:

- adaptive `k` provides a small but stable improvement on `room_dark_01`

---

## Adaptive k + feature refactor

- estimated poses: 555
- matched pairs: 547
- mean: 0.134606 m
- rmse: 0.146834 m
- max: 0.288260 m

Compared with adaptive `k` only:

- mean is almost unchanged: 0.134612 m -> 0.134606 m
- rmse is slightly worse: 0.146603 m -> 0.146834 m
- max is worse: 0.281849 m -> 0.288260 m

Conclusion:

- feature refactor remains stable
- however, it does not show a clear gain over adaptive `k` on `room_dark_01`

---

## Adaptive k + feature refactor + dynamic v3

- estimated poses: 555
- matched pairs: 547
- mean: 0.134596 m
- rmse: 0.146802 m
- max: 0.290208 m

Compared with adaptive `k` + feature refactor:

- mean improves only marginally: 0.134606 m -> 0.134596 m
- rmse improves only marginally: 0.146834 m -> 0.146802 m
- max becomes worse: 0.288260 m -> 0.290208 m

Compared with adaptive `k` only:

- mean is nearly unchanged
- rmse is slightly worse
- max is worse

Conclusion:

- dynamic v3 is successfully integrated and runnable
- on `room_dark_01`, it does not show a meaningful extra accuracy benefit

---

## Final conclusion for room_dark_01

For `room_dark_01`, the clearest positive result is the adaptive residual coefficient `k`.

- `adaptive k` is the most reliable improvement
- `feature refactor` does not clearly outperform `k`
- `dynamic v3` runs successfully but does not provide a meaningful gain on this mostly static indoor sequence
