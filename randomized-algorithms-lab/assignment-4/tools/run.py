"""Compile, test, schedule and audit C++ integration experiments."""
import argparse
import hashlib
import json
import platform
import random
import subprocess
import sys
from datetime import datetime
from pathlib import Path
from tempfile import TemporaryDirectory

from analyze import enrich, key, read, references, select, summarize, write

ROOT = Path(__file__).resolve().parents[1]
BUILD = ROOT/"build"
RESULT = ROOT/"results"
EXE = BUILD/"integral.exe"


def save(path, data):
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(json.dumps(data, ensure_ascii=False, indent=2)+"\n", encoding="utf-8")


def load(path):
    return json.loads(path.read_text(encoding="utf-8"))


def digest(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def call(args):
    return subprocess.run(list(map(str,args)), cwd=ROOT, check=True, timeout=1800,
                          capture_output=True, text=True, encoding="utf-8").stdout


def config(profile):
    cfg = load(ROOT/"config.json")
    if profile == "quick":
        cfg.update(sample_sizes=[256,1024], trials=4, warmups_per_setting=1,
                   confirmation_trials=4)
    return cfg


def build():
    BUILD.mkdir(exist_ok=True)
    commands = []
    sources = ["src/driver.cpp", "src/integral.hpp", "tests/test_integral.cpp"]
    for src, dest in [(sources[0],EXE),(sources[2],BUILD/"test_integral.exe")]:
        cmd = ["g++","-std=c++17","-O2","-Wall","-Wextra",src,"-o",str(dest)]
        call(cmd)
        commands.append(cmd)
    save(RESULT/"environment.json", dict(timestamp=datetime.now().astimezone().isoformat(),
         compiler=call(["g++","--version"]), commands=commands,
         os=platform.platform(), processor=platform.processor(), machine=platform.machine(),
         python=sys.version, engine="std::mt19937_64", executable_sha256=digest(EXE),
         source_sha256={p:digest(ROOT/p) for p in sources}))
    print("PASS build C++17 -O2", flush=True)


def test():
    cpp = call([BUILD/"test_integral.exe"]).strip()
    ref = references(config("formal"))
    with TemporaryDirectory(prefix="integration test ", dir=BUILD) as temp:
        jobs, core = Path(temp)/"jobs.csv", Path(temp)/"core.csv"
        valid = dict(a=0,b=4,N=131,K=64,trial_id=0,seed=42,warmup=0)
        write(jobs,[valid,valid])
        call([EXE,jobs,core])
        x,y = read(core)
        assert {k:v for k,v in x.items() if k != "elapsed_ns"} == {
            k:v for k,v in y.items() if k != "elapsed_ns"}
        assert int(x["evaluations"]) == valid["N"]
        invalid = [dict(a="nan"),dict(b="inf"),dict(b=0),dict(N=0),dict(N=1),
                   dict(N=-1),dict(K=0),dict(K=66),dict(seed=2**64),dict(warmup=2)]
        for change in invalid:
            write(jobs,[dict(valid,**change)])
            p = subprocess.run([str(EXE),str(jobs),str(core)], capture_output=True, text=True)
            assert p.returncode and "ERROR:" in p.stderr, change
    save(RESULT/"tests.json",dict(status="PASS",cpp=cpp,cli_invalid_cases=len(invalid),
         fixed_seed_cli=True, nondivisible_allocation=True, reference_checks=len(ref),
         max_reference_difference=max(r["abs_difference"] for r in ref)))
    print(cpp, flush=True)
    print(f"PASS {len(invalid)} CLI invalid cases, reproducibility and {len(ref)} reference checks", flush=True)


def settings(cfg):
    a,b = cfg["target"]
    return sorted({(a,b,n,1) for a,b in cfg["intervals"] for n in cfg["sample_sizes"]}
                  |{(a,b,n,k) for n in cfg["sample_sizes"] for k in cfg["strata"]})


def plan(cfg, configurations, previous_seeds=None):
    confirmation = previous_seeds is not None
    seed_rng = random.Random(cfg["confirmation_seed"] if confirmation else cfg["master_seed"])
    schedule = random.Random(cfg["confirmation_schedule_seed"] if confirmation else cfg["schedule_seed"])
    used = set(previous_seeds or ())
    jobs = []
    for warmup, count in [(1,cfg["warmups_per_setting"]),
                          (0,cfg["confirmation_trials"] if confirmation else cfg["trials"])]:
        phase = []
        for a,b,n,k in configurations:
            for trial in range(count):
                seed = seed_rng.getrandbits(64)
                while seed in used:
                    seed = seed_rng.getrandbits(64)
                used.add(seed)
                phase.append(dict(a=a,b=b,N=n,K=k,trial_id=trial,seed=seed,warmup=warmup))
        schedule.shuffle(phase)
        jobs.extend(phase)
    return jobs


def phase(out, cfg, name, jobs):
    job_path, core_path = out/f"{name}_jobs.csv",out/f"{name}_core.csv"
    write(job_path,jobs)
    cmd = [EXE,job_path,core_path]
    call(cmd)
    raw = enrich(read(core_path),jobs,cfg,name=="confirmation")
    summary = summarize(raw,cfg["relative_rmse_limit"])
    write(out/f"{name}_raw.csv",raw)
    write(out/f"{name}_summary.csv",summary)
    print(f"PASS {name}: {len(raw)} measured trials, {len(jobs)-len(raw)} warmups",flush=True)
    return summary, list(map(str,cmd))


def run(profile):
    cfg = config(profile)
    out = RESULT if profile=="formal" else RESULT/"quick"
    env = load(RESULT/"environment.json")
    assert digest(EXE)==env["executable_sha256"]
    assert all(digest(ROOT/p)==h for p,h in env["source_sha256"].items())
    write(out/"references.csv", references(cfg))
    main = plan(cfg,settings(cfg))
    summary,cmd = phase(out,cfg,"main",main)
    selection = select(summary,cfg)
    candidates = [key(r) for r in selection["candidates"]]
    commands = [cmd]
    if candidates:
        jobs = plan(cfg,candidates,{j["seed"] for j in main})
        _,cmd = phase(out,cfg,"confirmation",jobs)
        commands.append(cmd)
    else:
        for name in ("jobs","core","raw","summary"):
            (out/f"confirmation_{name}.csv").unlink(missing_ok=True)
        print("No qualifying configuration for independent confirmation", flush=True)
    save(out/"selection.json",selection)
    save(out/"run_manifest.json",dict(profile=profile, config=cfg, environment=env,
         commands=commands, overlap="A+B rows are measured once and shared by both analyses",
         candidate_rule="confirm all qualifying target settings within time margin of fastest",
         csv_sha256={p.name:digest(p) for p in sorted(out.glob("*.csv"))}))
    audit(profile)


def audit(profile):
    out = RESULT if profile=="formal" else RESULT/"quick"
    manifest = load(out/"run_manifest.json")
    cfg = manifest["config"]
    assert profile==manifest["profile"] and cfg==config(profile)
    assert all(digest(out/p)==h for p,h in manifest["csv_sha256"].items())

    def check_table(name,rows):
        assert read(out/f"{name}.csv")==[{k:str(v) for k,v in r.items()} for r in rows], name

    check_table("references",references(cfg))
    main = plan(cfg,settings(cfg))
    check_table("main_jobs",main)
    raw = enrich(read(out/"main_core.csv"),main,cfg)
    check_table("main_raw",raw)
    summary = summarize(raw,cfg["relative_rmse_limit"])
    check_table("main_summary",summary)
    selection = select(summary,cfg)
    assert selection==load(out/"selection.json")
    candidates = [key(r) for r in selection["candidates"]]
    checked = dict(main_rows=len(raw),main_settings=len(summary),overlap_rows=sum(r["experiment"]=="A+B" for r in raw))
    if candidates:
        jobs = plan(cfg,candidates,{j["seed"] for j in main})
        assert len({j["seed"] for j in main+jobs})==len(main+jobs)
        check_table("confirmation_jobs",jobs)
        raw = enrich(read(out/"confirmation_core.csv"),jobs,cfg,True)
        check_table("confirmation_raw",raw)
        check_table("confirmation_summary",summarize(raw,cfg["relative_rmse_limit"]))
        checked.update(confirmation_rows=len(raw),confirmation_settings=len(candidates))
    else:
        checked.update(confirmation_rows=0,confirmation_settings=0)
    save(out/"audit.json",dict(status="PASS",profile=profile,**checked))
    print(f"PASS {profile} CSV audit {checked}",flush=True)


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("action",choices=["build","test","run","audit","all"])
    parser.add_argument("--profile",choices=["quick","formal"],default="formal")
    args = parser.parse_args()
    if args.action in ("build","all"):
        build()
    if args.action in ("test","all"):
        test()
    if args.action in ("run","all"):
        run(args.profile)
    if args.action=="audit":
        audit(args.profile)


if __name__=="__main__":
    main()
