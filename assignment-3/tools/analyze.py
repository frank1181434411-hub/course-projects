"""Recompute summaries from actual samples and raw timing; audit saved tables."""
import argparse
import csv
import json
import math
import statistics
from collections import defaultdict
from pathlib import Path
import numpy as np
from run import ROOT,save,digest

def read(p):
    with p.open(encoding='utf-8',newline='') as f:return list(csv.DictReader(f))
def write(p,rows):
    with p.open('w',encoding='utf-8',newline='') as f:
        w=csv.DictWriter(f,fieldnames=list(rows[0]));w.writeheader();w.writerows(rows)
def calculate(out):
    manifest=json.loads((out/'run_manifest.json').read_text(encoding='utf-8'));cfg=manifest['config']
    stats=[];hist=[];qq=[];times=[]
    for d in manifest['distribution']:
        path=out/d['sample_file'];assert digest(path)==d['sha256']
        x=np.fromfile(path,dtype='<f8');n=len(x);assert n==d['n']==cfg['distribution_n'] and np.isfinite(x).all()
        if d['method']=='clt': assert np.max(np.abs(x))<=math.sqrt(3*d['m'])+1e-10
        mean=float(np.mean(x));v=float(np.mean((x-mean)**2));k=float(np.mean((x-mean)**4)/v**2-3)
        c2=int(np.count_nonzero(np.abs(x)>2));c3=int(np.count_nonzero(np.abs(x)>3))
        c,r=d['candidates'],d['rejected'];assert (c==n+r) if d['method']=='rejection' else c==r==0
        stats.append(dict(method=d['method'],m=d['m'],n=n,seed=d['seed'],mean=mean,variance=v,excess_kurtosis=k,
            count_abs_gt2=c2,rate_abs_gt2=c2/n,count_abs_gt3=c3,rate_abs_gt3=c3/n,candidates=c,rejected=r,
            acceptance_rate=n/c if c else '',attempts_per_output=c/n if c else ''))
        bins=np.linspace(-6,6,121);counts,_=np.histogram(x,bins)
        for i,count in enumerate(counts):hist.append(dict(method=d['method'],m=d['m'],left=float(bins[i]),right=float(bins[i+1]),count=int(count),density=float(count/(n*(bins[i+1]-bins[i])))))
        # 线性体部加几何尾部秩；保留最小值与最大值。
        tail=np.unique(np.rint(np.geomspace(1,max(2,n//100),300)).astype(int))
        ranks=np.unique(np.concatenate([np.rint(np.linspace(1,n,1401)).astype(int),tail,n+1-tail]))
        x.sort()
        for rank in ranks:
            p=(int(rank)-0.5)/n
            qq.append(dict(method=d['method'],m=d['m'],rank=int(rank),p=p,theoretical=statistics.NormalDist().inv_cdf(p),sample=float(x[rank-1])))
    groups=defaultdict(list)
    for rel,h in manifest['raw_sha256'].items():
        assert digest(out/rel)==h
        rows=read(out/rel);assert len(rows)==cfg['trials']
        assert [int(r['repeat']) for r in rows]==list(range(cfg['trials']))
        for r in rows:
            assert int(r['elapsed_ns'])>0 and math.isfinite(float(r['checksum']))
            groups[(r['method'],int(r['m']),int(r['n']))].append(int(r['elapsed_ns']))
    assert len(groups)==len(cfg['settings'])*len(cfg['sizes'])
    for (method,m,n),values in sorted(groups.items()):
        med=statistics.median(values)
        times.append(dict(method=method,m=m,n=n,trials=len(values),median_ns=med,median_ms=med/1e6,ns_per_sample=med/n))
    reference=[dict(distribution='N(0,1)',mean=0.0,variance=1.0,excess_kurtosis=0.0,
        rate_abs_gt2=math.erfc(2/math.sqrt(2)),rate_abs_gt3=math.erfc(3/math.sqrt(2)),
        rejection_acceptance_rate=math.sqrt(math.pi/(2*math.e)),rejection_attempts_per_output=math.sqrt(2*math.e/math.pi))]
    return dict(distribution_summary=stats,histogram=hist,qq=qq,timing_summary=times,theory_reference=reference)
def main():
    p=argparse.ArgumentParser();p.add_argument('--results',type=Path,default=ROOT/'results');p.add_argument('--audit',action='store_true');a=p.parse_args()
    tables=calculate(a.results)
    for name,rows in tables.items():
        path=a.results/(name+'.csv')
        if a.audit:
            old=read(path);assert len(old)==len(rows)
            for x,y in zip(old,rows):assert x=={k:str(v) for k,v in y.items()},(name,x,y)
        else:write(path,rows)
    save(a.results/'audit.json',dict(status='PASS',audit_saved_tables=a.audit,table_rows={k:len(v) for k,v in tables.items()},sample_hashes_and_counts=True,raw_hashes=True))
    print('PASS '+('audit' if a.audit else 'analyze')+' real samples, counts and timing tables')
if __name__=='__main__':main()
