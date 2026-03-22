# street_03 overall summary

## Evaluation setup

- dataset: `street_03`
- GT: `eval/formal/street_03/gt/street_03_gt_tum.txt`
- alignment: `--align`
- max time diff: `--t_max_diff 0.05`

---

## Baseline

- estimated poses: 1686
- matched pairs: 1686
- mean: 0.129208 m
- rmse: 0.139533 m
- max: 0.422916 m

Conclusion:

- baseline runs successfully on `street_03`
- synchronization and trajectory export are correct
- this result serves as the outdoor reference result

---

## Current combo

Current combo means:

- adaptive `k`
- feature refactor
- dynamic point cloud filtering v3

Result:

- estimated poses: 1683
- matched pairs: 1683
- mean: 0.163851 m
- rmse: 0.205268 m
- max: 0.698998 m

---

## Comparison

Compared with baseline:

- estimated poses: 1686 -> 1683
- matched pairs: 1686 -> 1683
- mean worsened from 0.129208 m to 0.163851 m
- rmse worsened from 0.139533 m to 0.205268 m
- max worsened from 0.422916 m to 0.698998 m

Detailed difference:

- mean: +0.034643 m
- rmse: +0.065735 m
- max: +0.276082 m

---

## Conclusion

On `street_03`, the current combo version is clearly worse than the baseline.

This means:

- the full current combination does not transfer well to this outdoor sequence
- the current integrated version cannot yet be considered a robust improvement in outdoor dynamic scenarios

At the current stage, the experiment only supports the following conclusion:

- indoor static-like sequence (`room_dark_01`): adaptive `k` gives a small but stable gain
- outdoor street sequence (`street_03`): the current combined pipeline (`k + feature refactor + dynamic v3`) degrades accuracy relative to baseline

Therefore, the outdoor result should be reported honestly as a negative result for the current combination.
