"""Plot shared scales from audited experiment tables."""
import argparse
import os
from pathlib import Path
from run import ROOT,save
os.environ.setdefault('MPLCONFIGDIR',str(ROOT/'.mplconfig'))
os.environ.setdefault('MPLBACKEND','Agg')
import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
from matplotlib import font_manager
import numpy as np
from analyze import read

def label(method,m):return f'CLT m={m}' if method=='clt' else {'box_muller':'Box–Muller','rejection':'指数拒绝采样'}[method]
def main():
    p=argparse.ArgumentParser();p.add_argument('--results',type=Path,default=ROOT/'results');a=p.parse_args();out=a.results/'figures';out.mkdir(exist_ok=True)
    font=Path('C:/Windows/Fonts/msyh.ttc');assert font.exists()
    font_manager.fontManager.addfont(str(font));plt.rcParams.update({'font.family':font_manager.FontProperties(fname=str(font)).get_name(),'axes.unicode_minus':False,'figure.dpi':140,'font.size':10})
    stats=read(a.results/'distribution_summary.csv');hist=read(a.results/'histogram.csv');qq=read(a.results/'qq.csv');timing=read(a.results/'timing_summary.csv')
    def finish(fig,name):fig.tight_layout();fig.savefig(out/name,bbox_inches='tight');plt.close(fig)
    grid=np.linspace(-6,6,601);pdf=np.exp(-grid**2/2)/np.sqrt(2*np.pi)
    for name,data in [('density.png',hist),('qq.png',qq)]:
        fig,axes=plt.subplots(2,3,figsize=(13,8),sharex=True,sharey=True)
        for ax,s in zip(axes.flat,stats):
            rows=[r for r in data if r['method']==s['method'] and r['m']==s['m']]
            if name=='density.png':
                ax.bar([float(r['left']) for r in rows],[float(r['density']) for r in rows],width=.1,align='edge',alpha=.65,label='实测密度')
                ax.plot(grid,pdf,'r-',lw=1.2,label='N(0,1)');ax.set(xlim=(-6,6),ylim=(0,.45),xlabel='样本值',ylabel='密度')
            else:
                ax.scatter([float(r['theoretical']) for r in rows],[float(r['sample']) for r in rows],s=5,alpha=.65,label='样本分位数')
                ax.plot([-5.5,5.5],[-5.5,5.5],'r--',lw=1,label='y=x');ax.set(xlim=(-5.5,5.5),ylim=(-5.5,5.5),xlabel='标准正态理论分位数',ylabel='原始样本分位数');ax.set_aspect('equal')
            ax.set_title(label(s['method'],s['m']));ax.grid(alpha=.2);ax.legend(fontsize=8)
        axes.flat[-1].axis('off');finish(fig,name)
    fig,axes=plt.subplots(1,2,figsize=(12,4.5))
    for s in stats:
        rows=sorted([r for r in timing if r['method']==s['method'] and r['m']==s['m']],key=lambda r:int(r['n']))
        n=[int(r['n']) for r in rows];name=label(s['method'],s['m'])
        axes[0].plot(n,[float(r['median_ms']) for r in rows],'o-',label=name)
        axes[1].plot(n,[float(r['ns_per_sample']) for r in rows],'o-',label=name)
    for ax in axes:ax.set_xscale('log');ax.set_xlabel('输出样本数 N');ax.grid(alpha=.25);ax.legend(fontsize=8)
    axes[0].set(yscale='log',ylabel='中位耗时 (ms，对数)',title='生成总耗时（含所有拒绝尝试）');axes[1].set(ylabel='ns/sample',title='每个输出的中位耗时');finish(fig,'speed.png')
    from PIL import Image,ImageOps,ImageDraw
    thumbs=[]
    for name in ['density.png','qq.png','speed.png']:
        im=Image.open(out/name).convert('RGB');im.thumbnail((1050,650));thumbs.append(im)
    sheet=Image.new('RGB',(1050,sum(im.height for im in thumbs)),'white');y=0
    for im in thumbs:sheet.paste(im,(0,y));y+=im.height
    sheet.save(out/'inspection_sheet.png')
    save(out/'plot_environment.json',dict(matplotlib=matplotlib.__version__,numpy=np.__version__,font=str(font),histogram_range=[-6,6],bin_width=.1,qq_rule='unique rounded ranks: 1401 linear + 300 geometric in each 1% tail, including extrema'))
    print('PASS 3 figures and inspection sheet')
if __name__=='__main__':main()
