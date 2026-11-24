"""Plot the integrand and the measured, audited comparisons."""
import argparse
import os
from pathlib import Path

from analyze import read
from run import RESULT, ROOT, load, save

os.environ.setdefault("MPLCONFIGDIR",str(ROOT/".mplconfig"))
import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt
import numpy as np
from matplotlib import font_manager
from matplotlib.ticker import FuncFormatter, NullFormatter, StrMethodFormatter

FIGURES = ("integrand.png","range_rmse.png","strata_rmse.png","accuracy_time.png")


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--profile",choices=["quick","formal"],default="formal")
    args = parser.parse_args()
    path = RESULT if args.profile=="formal" else RESULT/"quick"
    assert load(path/"audit.json")["status"]=="PASS"
    cfg = load(path/"run_manifest.json")["config"]
    data = read(path/"main_summary.csv")
    out = path/"figures"
    out.mkdir(exist_ok=True)
    font = Path("C:/Windows/Fonts/msyh.ttc")
    font_manager.fontManager.addfont(str(font))
    plt.rcParams.update({"font.family":font_manager.FontProperties(fname=str(font)).get_name(),
                         "axes.unicode_minus":False,"figure.dpi":145,"font.size":10})
    colors = plt.get_cmap("tab10").colors
    limit = cfg["relative_rmse_limit"]

    def finish(fig, name):
        fig.tight_layout()
        fig.savefig(out/name,bbox_inches="tight")
        plt.close(fig)

    fig, axes = plt.subplots(2,2,figsize=(8.2,4.6),sharex=True,sharey=True)
    x = np.linspace(0,4,401)
    for i, (ax,(a,b)) in enumerate(zip(axes.flat,cfg["intervals"])):
        selected = np.linspace(a,b,200)
        ax.plot(x,np.exp(-x*x),color="#334155",lw=1.4)
        ax.fill_between(selected,np.exp(-selected*selected),color=colors[i],alpha=.36)
        ax.axvline(b,color=colors[i],ls=":",lw=1)
        ax.set_title(f"[{a:g}, {b:g}]")
        ax.grid(alpha=.18)
    fig.suptitle("被积函数 exp(−x²) 与四种积分范围")
    fig.supxlabel("x")
    fig.supylabel("f(x)")
    finish(fig,FIGURES[0])

    def rmse_axes(ax, x_label):
        ax.set(xscale="log",yscale="log",xlabel=x_label,ylabel="相对 RMSE")
        ax.yaxis.set_major_formatter(FuncFormatter(lambda y,_: f"{100*y:g}%"))
        ax.yaxis.set_minor_formatter(NullFormatter())
        ax.axhline(limit,color="#374151",ls="--",lw=1,label=f"{limit:.0%} 阈值")
        ax.grid(which="both",alpha=.20)

    fig, ax = plt.subplots(figsize=(7.6,4.4))
    for i,(a,b) in enumerate(cfg["intervals"]):
        values = [r for r in data if int(r["K"])==1 and (float(r["a"]),float(r["b"]))==(a,b)]
        ax.plot([int(r["N"]) for r in values],[float(r["relative_rmse"]) for r in values],
                "o-",color=colors[i],label=f"[{a:g}, {b:g}]")
    rmse_axes(ax,"函数求值次数 N")
    ax.set(title="普通均值法：积分范围与误差",xticks=cfg["sample_sizes"])
    ax.xaxis.set_major_formatter(StrMethodFormatter("{x:,.0f}"))
    ax.legend()
    finish(fig,FIGURES[1])

    target = [r for r in data if [float(r["a"]),float(r["b"])]==cfg["target"]]
    for name,by_time in [(FIGURES[2],False),(FIGURES[3],True)]:
        fig, ax = plt.subplots(figsize=(7.6,4.4))
        for i,k in enumerate(cfg["strata"]):
            values = [r for r in target if int(r["K"])==k]
            xs = [float(r["median_ns"])/1e6 if by_time else int(r["N"]) for r in values]
            ys = [float(r["relative_rmse"]) for r in values]
            ax.plot(xs,ys,"o-",color=colors[i],label=f"K={k}，h={float(values[0]['h']):g}")
        rmse_axes(ax,"中位核心耗时（ms）" if by_time else "总函数求值次数 N")
        ax.set_title("[0,4]：精度与耗时" if by_time else "[0,4]：分层数与误差")
        if by_time:
            ax.xaxis.set_major_formatter(FuncFormatter(lambda x,_: f"{x:g}"))
        else:
            ax.set_xticks(cfg["sample_sizes"])
            ax.xaxis.set_major_formatter(StrMethodFormatter("{x:,.0f}"))
        ax.legend()
        finish(fig,name)
    save(out/"plot_environment.json",dict(backend=matplotlib.get_backend(),matplotlib=matplotlib.__version__,
         numpy=np.__version__,font=str(font),figures=FIGURES))
    print(f"PASS {args.profile} plots: {len(FIGURES)} PNGs")


if __name__=="__main__":
    main()
