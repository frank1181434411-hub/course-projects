"""Generate the Chinese report from the audited formal experiments."""
import math
from statistics import fmean

from analyze import key, read
from run import RESULT, ROOT, load


def table(headers, rows):
    return "\n".join("| "+" | ".join(map(str,r))+" |" for r in
                     [headers,["---"]*len(headers),*rows])


def percent(x, digits=4):
    return f"{float(x):.{digits}%}"


def slope(rows):
    xs = [math.log(int(r["N"])) for r in rows]
    ys = [math.log(float(r["relative_rmse"])) for r in rows]
    x, y = fmean(xs), fmean(ys)
    return sum((i-x)*(j-y) for i,j in zip(xs,ys))/sum((i-x)**2 for i in xs)


def main():
    manifest = load(RESULT/"run_manifest.json")
    audit = load(RESULT/"audit.json")
    tests = load(RESULT/"tests.json")
    assert manifest["profile"]==audit["profile"]=="formal"
    assert audit["status"]==tests["status"]=="PASS"
    cfg, env = manifest["config"], manifest["environment"]
    main_data = read(RESULT/"main_summary.csv")
    iid = [r for r in main_data if int(r["K"])==1]
    target = [r for r in main_data if [float(r["a"]),float(r["b"])]==cfg["target"]]
    by_key = {key(r):r for r in target}
    selection = load(RESULT/"selection.json")
    limit = percent(cfg["relative_rmse_limit"],0)
    ref_table = table(["区间","erf 参考值","自适应 Simpson","绝对差"],
                      [[f"[{float(r['a']):g},{float(r['b']):g}]",f"{float(r['erf_reference']):.16f}",
                        f"{float(r['adaptive_simpson']):.16f}",f"{float(r['abs_difference']):.2e}"]
                       for r in read(RESULT/"references.csv")])
    ranges = []
    for item in selection["ranges"]:
        a,b = item["a"],item["b"]
        rows = [r for r in iid if key(r)[:2]==(a,b)]
        ranges.append([f"[{a:g},{b:g}]",*(percent(r["relative_rmse"]) for r in rows),
                       f"{item['min_N']:,}" if item["min_N"] else "未达标",f"{slope(rows):.3f}"])
    range_table = table(["区间",*(f"N={n:,}" for n in cfg["sample_sizes"]),"最小达标 N","斜率"],ranges)
    strata_table = table(["N","K","h","相对 RMSE","平均 SE","覆盖率","中位耗时（ms）","ns/sample","达标"],
                         [[f"{int(r['N']):,}",r["K"],f"{float(r['h']):g}",percent(r["relative_rmse"]),
                           f"{float(r['mean_se']):.3g}",percent(r["coverage"],0),
                           f"{float(r['median_ns'])/1e6:.5f}",f"{float(r['ns_per_sample']):.2f}",
                           "是" if int(r["meets_limit"]) else "否"] for r in target])
    confirmations = read(RESULT/"confirmation_summary.csv") if selection["candidates"] else []
    confirm_table = table(["N","K","主实验 RMSE（相对）","复验 RMSE（相对）","主实验 μs","复验 μs","复验覆盖率","复验达标"],
                          [[r["N"],r["K"],percent(by_key[key(r)]["relative_rmse"]),percent(r["relative_rmse"]),
                            f"{float(by_key[key(r)]['median_ns'])/1000:.1f}",f"{float(r['median_ns'])/1000:.1f}",
                            percent(r["coverage"],0),"是" if int(r["meets_limit"]) else "否"]
                           for r in confirmations]) if confirmations else "测试范围内没有可供复验的达标配置。"
    if confirmations:
        fastest = selection["candidates"][0]
        validated = [r for r in confirmations if int(r["meets_limit"])]
        result = (f"主实验在 {len(selection['eligible'])} 个达标配置中，"
                  f"N={fastest['N']:,}、K={fastest['K']} 的中位耗时最低，为 {fastest['median_ns']/1000:.1f} μs。"
                  f"共 {len(confirmations)} 组进入独立复验，其中 {len(validated)} 组达标。")
        if validated:
            quickest = min(validated,key=lambda r:float(by_key[key(r)]["median_ns"]))
            accurate = min(validated,key=lambda r:float(r["relative_rmse"]))
            result += f"按主实验中位耗时，优先试用 N={int(quickest['N']):,}、K={quickest['K']}（h={quickest['h']}）。"
            if key(accurate)!=key(quickest):
                gap = float(by_key[key(accurate)]["median_ns"])/float(by_key[key(quickest)]["median_ns"])-1
                result += (f"更重视精度时可选 N={int(accurate['N']):,}、K={accurate['K']}（h={accurate['h']}）。"
                           f"后者主实验中位耗时相对前者高 {gap:.1%}，复验相对 RMSE 为 {percent(accurate['relative_rmse'])}。"
                           "这点耗时差不足以确认唯一赢家。")
        else:
            result += "候选均未通过独立复验，本轮没有经复验确认的推荐。"
    else:
        result = "测试范围内没有达标设置。"
    coverages = [float(r["coverage"]) for r in main_data]
    macro = dict(ref_table=ref_table,range_table=range_table,strata_table=strata_table,
                 confirm_table=confirm_table,recommendation=result,
                 compiler=env["compiler"].splitlines()[0], os=env["os"], machine=env["machine"],
                 processor=env["processor"], python=env["python"].split()[0],limit=limit,
                 seed=cfg["master_seed"],schedule=cfg["schedule_seed"],
                 confirmation_seed=cfg["confirmation_seed"],
                 confirmation_schedule=cfg["confirmation_schedule_seed"],
                 core_checks=tests["cpp"],cli_checks=tests["cli_invalid_cases"],
                 max_difference=f"{tests['max_reference_difference']:.2e}",
                 main_rows=audit["main_rows"],main_settings=audit["main_settings"],
                 overlap=audit["overlap_rows"],confirmation_rows=audit["confirmation_rows"],
                 cover_low=percent(min(coverages),0),cover_high=percent(max(coverages),0))
    text = (ROOT/"docs/报告模板.md").read_text(encoding="utf-8")
    for name,value in macro.items():
        token = "@"+name.upper()+"@"
        assert token in text, token
        text = text.replace(token,str(value))
    assert not any(word.startswith("@") and word.endswith("@") for word in text.split())
    (ROOT/"docs/实验报告.md").write_text(text,encoding="utf-8")
    print("PASS report generated from formal results")


if __name__=="__main__":
    main()
