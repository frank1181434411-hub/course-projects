"""Build, test and run the real C++ experiments; no synthetic measurements."""
import argparse
import csv
import hashlib
import json
import os
import platform
import random
import subprocess
import sys
from datetime import datetime, timezone
from pathlib import Path

ROOT=Path(__file__).resolve().parents[1]
EXE=ROOT/'build/normal.exe'
def save(path,value):
    path.parent.mkdir(parents=True,exist_ok=True)
    path.write_text(json.dumps(value,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
def digest(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()
def call(args):
    p=subprocess.run(list(map(str,args)),cwd=ROOT,capture_output=True,text=True,encoding='utf-8',errors='replace',timeout=1800)
    if p.returncode: raise RuntimeError(p.stderr or p.stdout)
    return p.stdout
def build():
    (ROOT/'build').mkdir(exist_ok=True)
    commands=[]
    for src,name in [('src/driver.cpp','normal.exe'),('tests/test_normal.cpp','test_normal.exe')]:
        cmd=['g++','-std=c++17','-O2','-Wall','-Wextra',src,'-o',str(ROOT/'build'/name)]
        call(cmd);commands.append(cmd)
    save(ROOT/'results/environment.json',dict(timestamp_utc=datetime.now(timezone.utc).isoformat(),commands=commands,
        compiler=call(['g++','--version']),optimization='-O2',os=platform.platform(),machine=platform.machine(),
        processor=platform.processor(),python=sys.version,byteorder=sys.byteorder,
        executable_sha256=digest(EXE),source_sha256={str(p.relative_to(ROOT)):digest(p) for p in (ROOT/'src').glob('*')},
        engine='std::mt19937_64',uniform='(double(rng() >> 12) + 0.5) * 2^-52; open (0,1) midpoint grid'))
    print('PASS build C++17 -O2')
def args(mode,method,m,n,seed,output,trials=1,warmups=0):
    return [EXE,'--mode',mode,'--method',method,'--m',m,'--n',n,'--seed',seed,'--trials',trials,'--warmups',warmups,'--output',output]
def test():
    import tempfile
    result=call([ROOT/'build/test_normal.exe'])
    with tempfile.TemporaryDirectory(prefix='normal checks with spaces ',dir=ROOT/'build') as tmp:
        out=Path(tmp)/'sample with spaces.bin'
        cmd=list(map(str,args('distribution','box_muller',12,3,42,out)))
        call(cmd);first=out.read_bytes();call(cmd);assert first==out.read_bytes() and len(first)==24
        cases=[('--n','0'),('--n','-1'),('--n','10000001'),('--m','0'),('--m','-1'),('--m','1000001'),('--seed','18446744073709551616'),('--seed','1x'),('--method','bad'),('--mode','bad'),('--trials','0')]
        for key,value in cases:
            bad=cmd.copy();bad[bad.index(key)+1]=value
            p=subprocess.run(bad,capture_output=True,text=True);assert p.returncode!=0 and 'ERROR:' in p.stderr
        for bad in [cmd[:-1],cmd+['--n','2'],cmd+['--unknown','1']]:
            assert subprocess.run(bad,capture_output=True).returncode!=0
    save(ROOT/'results/tests.json',dict(status='PASS',cpp=result.strip(),cli_invalid_cases=len(cases)+3,path_with_spaces=True,binary_reproducibility=True))
    print(result.strip());print('PASS 14 CLI invalid cases, path with spaces, binary reproducibility')
def run(profile):
    cfg=json.loads((ROOT/'config.json').read_text())
    if profile=='quick': cfg.update(distribution_n=10000,sizes=[100,1000],trials=3,warmups=1)
    out=ROOT/'results' if profile=='formal' else ROOT/'results/quick'
    (out/'samples').mkdir(parents=True,exist_ok=True);(out/'raw').mkdir(exist_ok=True)
    env=json.loads((ROOT/'results/environment.json').read_text(encoding='utf-8'))
    assert digest(EXE)==env['executable_sha256']
    assert all(digest(ROOT/p)==h for p,h in env['source_sha256'].items())
    rng=random.Random(cfg['master_seed']);schedule=random.Random(cfg['schedule_seed']);jobs=[];dist=[];records=[]
    for method,m in cfg['settings']:
        seed=rng.getrandbits(64);path=out/'samples'/f'{method}_{m}.bin'
        cmd=args('distribution',method,m,cfg['distribution_n'],seed,path)
        count=json.loads(call(cmd));dist.append(dict(method=method,m=m,n=cfg['distribution_n'],seed=seed,**{k:count[k] for k in ['candidates','rejected']},sample_file=str(path.relative_to(out)),sha256=digest(path)))
        records.append(list(map(str,cmd)))
        for n in cfg['sizes']: jobs.append((method,m,n,rng.getrandbits(64)))
    schedule.shuffle(jobs)
    # 串行测量；绘图和统计在所有性能任务完成后执行。
    for method,m,n,seed in jobs:
        path=out/'raw'/f'{method}_{m}_{n}.csv';cmd=args('performance',method,m,n,seed,path,cfg['trials'],cfg['warmups'])
        call(cmd);records.append(list(map(str,cmd)));print(f'{profile}: {method} m={m} n={n} done',flush=True)
    save(out/'run_manifest.json',dict(profile=profile,config=cfg,distribution=dist,commands=records,environment=env,
        raw_sha256={str(p.relative_to(out)):digest(p) for p in (out/'raw').glob('*.csv')}))
    print('PASS real experiments '+profile)
def main():
    p=argparse.ArgumentParser();p.add_argument('action',choices=['build','test','run','all']);p.add_argument('--profile',choices=['quick','formal'],default='formal');a=p.parse_args()
    if a.action in ['build','all']:build()
    if a.action in ['test','all']:test()
    if a.action in ['run','all']:run(a.profile)
if __name__=='__main__':main()
