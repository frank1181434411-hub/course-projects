# 蒙特卡洛积分实验

在四种范围上用普通均值法估计 exp(−x²) 的积分。在固定 [0,4] 上比较等宽分层，每组使用相同的总函数求值预算。主实验与复验的相对 RMSE 门槛为 1%。

[中文报告](docs/实验报告.md) · [PDF](docs/实验报告.pdf) · [主实验汇总](results/main_summary.csv) · [独立复验](results/confirmation_summary.csv)

## 编译、测试与复现

使用 Windows PowerShell。实验调度与统计用 Python 标准库，画图复用作业一的 matplotlib 环境，PDF 复用作业三的 Markdown 与 Edge 导出流程。

```powershell
Set-Location 'D:/CPP/random/assignment-4'
$env:PYTHONIOENCODING = 'utf-8'
$plotPython = '..\assignment-1\.plot-venv\Scripts\python.exe'

# 快速流程
python tools/run.py all --profile quick
& $plotPython tools/plot.py --profile quick

# 正式主实验、候选复验与审计
python tools/run.py run --profile formal
python tools/run.py audit --profile formal
& $plotPython tools/plot.py --profile formal
python tools/report.py
& $plotPython tools/export_pdf.py
```

`run.py build` 和 `run.py test` 可独立调用。`run.py all` 默认使用正式模式。快速模式只用于验证流程，输出在 `results/quick/`。正式命令会重跑计时并覆盖对应 CSV，复验对象由本轮结果按配置中的耗时门槛选出。

若复用的环境不存在，可改用已有 matplotlib 和 NumPy 的 Python。PDF 另外需要 markdown2 与 Edge。导出脚本默认查找作业三的 `build/pdf-tools`，也支持 `--pdf-tools` 和 `--edge` 指定路径。

## 目录与字段

| 位置 | 内容 |
|---|---|
| [src/integral.hpp](src/integral.hpp) | 均匀数映射、Welford 累计、等宽分层，K=1 即普通均值法 |
| [src/driver.cpp](src/driver.cpp) | CSV 批处理、核心计时、输出校验 |
| [tests/test_integral.cpp](tests/test_integral.cpp) | 常数与线性函数、固定种子、层内方差、余数分配和非法参数 |
| [config.json](config.json) | 两组范围、总 N、K、重复次数、种子和事先固定的门槛 |
| [results/references.csv](results/references.csv) | math.erf 与自适应 Simpson 参考值及容差 |
| [results/main_jobs.csv](results/main_jobs.csv) / [main_core.csv](results/main_core.csv) | 执行顺序、预热、每次实际种子、C++ 原始结果 |
| [results/main_raw.csv](results/main_raw.csv) / [main_summary.csv](results/main_summary.csv) | 独立测量明细及分组汇总 |
| [results/selection.json](results/selection.json) | 每范围最小达标 N、固定目标下达标组和复验候选 |
| [results/confirmation_raw.csv](results/confirmation_raw.csv) / [confirmation_summary.csv](results/confirmation_summary.csv) | 独立新种子复验的明细与汇总，其他 `confirmation_*` 字段同主实验 |
| [results/run_manifest.json](results/run_manifest.json) / [audit.json](results/audit.json) | 实际构建与运行信息、配置、CSV 哈希与一致性检查 |

测量明细中的 `experiment=A+B` 为两组实验共用的 [0,4] 普通均值结果，每次只执行一次。`N` 是总函数求值次数，`h=(b-a)/K`。预热在任务表和核心输出中标为 1，不参与统计。trial_id 从 0 起在各配置、各阶段单独编号。

`variance` 是积分估计的方差估计，`standard_error` 为其平方根。`ci_low`、`ci_high` 对应估计值 ±1.96 SE。`covered=1` 表示参考值落在闭区间内。`elapsed_ns` 只含 C++ 核心，`ns_per_sample` 为中位核心耗时除以 N。偏差为跨重复估计均值减去参考值。

驱动也可直接运行 `build/integral.exe jobs.csv core.csv`。输入表头为 `a,b,N,K,trial_id,seed,warmup`，端点须有限且 a<b，N 为 2..10,000,000，K 为 1..floor(N/2)，warmup 为 0 或 1，种子为无符号 64 位整数。输出保留估计值、估计方差、SE、实际求值次数和计时。

## 已测结果

普通均值法四种范围的最小达标 N 依次为 1,000、1,000、10,000、100,000。固定 [0,4] 时，N=1,000、K=16 的主实验中位耗时最低，K=64 多用约 4% 的时间并获得更小误差。独立复验相对 RMSE 分别为 0.3857% 和 0.1021%，均达标。

[函数与四种范围](results/figures/integrand.png) · [范围与相对 RMSE](results/figures/range_rmse.png) · [分层与相对 RMSE](results/figures/strata_rmse.png) · [精度与耗时](results/figures/accuracy_time.png)
