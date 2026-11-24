"""Reference checks and statistics outside the C++ timed region."""
import csv
import math
from collections import defaultdict
from statistics import fmean, median


def read(path):
    with path.open(encoding="utf-8", newline="") as file:
        return list(csv.DictReader(file))


def write(path, rows):
    if not rows:
        raise ValueError(f"empty table: {path}")
    path.parent.mkdir(parents=True, exist_ok=True)
    with path.open("w", encoding="utf-8", newline="") as file:
        writer = csv.DictWriter(file, fieldnames=list(rows[0]))
        writer.writeheader()
        writer.writerows(rows)


def reference(a, b):
    return math.sqrt(math.pi) / 2 * (math.erf(b) - math.erf(a))


def adaptive_simpson(a, b, tolerance):
    def f(x):
        return math.exp(-x*x)

    def area(left, right, fl, fm, fr):
        return (right-left) / 6 * (fl+4*fm+fr)

    def split(left, right, fl, fm, fr, whole, tol, depth):
        mid = (left+right)/2
        f1, f2 = f((left+mid)/2), f((mid+right)/2)
        s1, s2 = area(left, mid, fl, f1, fm), area(mid, right, fm, f2, fr)
        delta = s1+s2-whole
        if abs(delta) <= 15*tol:
            return s1+s2+delta/15
        if depth == 0:
            raise ArithmeticError("adaptive Simpson reached depth limit")
        return (split(left, mid, fl, f1, fm, s1, tol/2, depth-1)
                +split(mid, right, fm, f2, fr, s2, tol/2, depth-1))

    fa, fm, fb = f(a), f((a+b)/2), f(b)
    return split(a, b, fa, fm, fb, area(a, b, fa, fm, fb), tolerance, 30)


def references(cfg):
    rows = []
    for a, b in cfg["intervals"]:
        ref = reference(a, b)
        simpson = adaptive_simpson(a, b, cfg["simpson_tolerance"])
        error = abs(ref-simpson)
        assert error <= cfg["reference_agreement_tolerance"]
        rows.append(dict(a=a, b=b, erf_reference=ref, adaptive_simpson=simpson,
                         abs_difference=error, simpson_tolerance=cfg["simpson_tolerance"],
                         agreement_tolerance=cfg["reference_agreement_tolerance"]))
    return rows


def key(row):
    return float(row["a"]), float(row["b"]), int(row["N"]), int(row["K"])


def enrich(core, jobs, cfg, confirmation=False):
    assert len(core) == len(jobs)
    rows = []
    for r, j in zip(core, jobs):
        a, b, n, k = key(r)
        assert key(j) == key(r)
        for field in ("trial_id", "seed", "warmup"):
            assert int(r[field]) == int(j[field]), (field, r, j)
        assert int(r["evaluations"]) == n
        estimate, variance, se = map(float, (r["estimate"], r["variance"], r["standard_error"]))
        assert all(map(math.isfinite, (estimate, variance, se)))
        assert 0 <= estimate <= (b-a)*(1+1e-13) and variance >= 0 and se >= 0
        assert math.isclose(se*se, variance, rel_tol=1e-14)
        elapsed = int(r["elapsed_ns"])
        assert elapsed > 0
        if int(j["warmup"]):
            continue
        ref = reference(a, b)
        lo, hi = estimate-1.96*se, estimate+1.96*se
        label = "confirmation" if confirmation else ("A+B" if k == 1 and [a,b] == cfg["target"]
                                                    else "A" if k == 1 else "B")
        rows.append(dict(experiment=label, method="iid" if k == 1 else "stratified",
                         a=a, b=b, N=n, K=k, h=(b-a)/k, trial_id=int(r["trial_id"]),
                         seed=int(r["seed"]), estimate=estimate, reference=ref,
                         abs_error=abs(estimate-ref), rel_error=abs(estimate-ref)/abs(ref),
                         variance=variance, standard_error=se, ci_low=lo, ci_high=hi,
                         covered=int(lo <= ref <= hi), evaluations=n, elapsed_ns=elapsed))
    return rows


def summarize(rows, limit):
    groups = defaultdict(list)
    for row in rows:
        groups[key(row)].append(row)
    summary = []
    for (a,b,n,k), values in sorted(groups.items()):
        ref = values[0]["reference"]
        estimates = [x["estimate"] for x in values]
        mae = fmean(x["abs_error"] for x in values)
        rmse = math.sqrt(fmean(x["abs_error"]**2 for x in values))
        duration = median(x["elapsed_ns"] for x in values)
        summary.append(dict(experiment=values[0]["experiment"], method=values[0]["method"],
                            a=a, b=b, N=n, K=k, h=(b-a)/k, trials=len(values), reference=ref,
                            mean_estimate=fmean(estimates), bias=fmean(estimates)-ref, mae=mae,
                            rmse=rmse, relative_rmse=rmse/abs(ref),
                            mean_se=fmean(x["standard_error"] for x in values),
                            coverage=fmean(x["covered"] for x in values),
                            median_ns=duration, ns_per_sample=duration/n,
                            meets_limit=int(rmse/abs(ref) <= limit)))
    return summary


def select(summary, cfg):
    ranges = []
    for a, b in cfg["intervals"]:
        setting = [r for r in summary if r["K"] == 1 and (r["a"],r["b"]) == (a,b)]
        passed = [r for r in setting if r["meets_limit"]]
        ranges.append(dict(a=a, b=b, min_N=min((r["N"] for r in passed), default=None)))
    eligible = sorted((r for r in summary if [r["a"],r["b"]] == cfg["target"] and r["meets_limit"]),
                      key=lambda r: (r["median_ns"],r["N"],r["K"]))
    cutoff = eligible[0]["median_ns"]*(1+cfg["candidate_time_margin"]) if eligible else None
    candidates = [r for r in eligible if r["median_ns"] <= cutoff] if eligible else []
    return dict(ranges=ranges, elapsed_cutoff_ns=cutoff, eligible=eligible, candidates=candidates)
