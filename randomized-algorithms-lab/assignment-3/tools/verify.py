"""Final delivery checks: samples/tables, report rendering inputs and image decoding."""
import json
import re
from pathlib import Path
from PIL import Image
from analyze import calculate,read
from report import render
from run import ROOT,save,digest

def main():
    out=ROOT/'results';tables=calculate(out)
    for name,rows in tables.items():
        assert read(out/(name+'.csv'))==[{k:str(v) for k,v in r.items()} for r in rows]
    report=ROOT/'docs/实验报告.md';text=report.read_text(encoding='utf-8');assert text==render()
    links=re.findall(r'\]\(([^)]+)\)',text)
    for link in links:assert (report.parent/link).is_file(),link
    images={}
    for path in (out/'figures').glob('*.png'):
        with Image.open(path) as im:
            images[path.name]=dict(size=list(im.size),sha256=digest(path));im.verify()
    assert {'density.png','qq.png','speed.png','inspection_sheet.png'}==set(images)
    hist=read(out/'histogram.csv');qq=read(out/'qq.csv')
    for d in read(out/'distribution_summary.csv'):
        match=lambda r:r['method']==d['method'] and r['m']==d['m']
        h=[r for r in hist if match(r)];q=[r for r in qq if match(r)]
        assert len(h)==120 and sum(int(r['count']) for r in h)==int(d['n'])
        assert int(q[0]['rank'])==1 and int(q[-1]['rank'])==int(d['n'])
        assert all(abs(float(r['sample']))<=5.5 for r in q)
    manifest=json.loads((out/'run_manifest.json').read_text(encoding='utf-8'))
    assert manifest['profile']=='formal' and manifest['config']['trials']==20
    assert sum(len(read(out/rel)) for rel in manifest['raw_sha256'])==300
    assert sum(d['n'] for d in manifest['distribution'])==5000000
    save(out/'delivery_checks.json',dict(status='PASS',formal_outputs=5000000,formal_timing_records=300,
        all_tables_recomputed=True,report_matches_csv=True,report_links_checked=links,images=images,
        histogram_samples_outside_plot=0,qq_extrema_preserved=True,pdf_generated=False))
    print('PASS delivery: 5,000,000 outputs, 300 timings, all tables/report/links/images consistent')
if __name__=='__main__':main()
