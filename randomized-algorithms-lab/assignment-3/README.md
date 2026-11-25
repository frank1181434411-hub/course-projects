# 正态随机变量的生成

比较 CLT m=3、12、48、Box–Muller 与指数拒绝采样。已实际编译、测试、运行快速与正式实验，并由实测数据生成中文 Markdown 报告；沿用前两次格式，未导出 PDF。

[中文报告](docs/实验报告.md) · [密度图](results/figures/density.png) · [Q–Q 图](results/figures/qq.png) · [速度图](results/figures/speed.png)

## 目录

```text
src/normal.hpp          三种变换、统一均匀数映射
src/driver.cpp          C++17 驱动、核心计时、输出校验
tests/test_normal.cpp  确定性分支与功能检查
tools/run.py            构建、测试、快速/正式调度
tools/analyze.py        实测样本统计、原始计时汇总与一致性审计
tools/plot.py           读取汇总 CSV 绘制中文图
tools/report.py         从正式 CSV 生成/检查报告
config.json             正式参数，不因快速运行而改写
results/raw/            原始计时 CSV
results/figures/        三张图与可视检查拼图
results/samples/        可再生二进制样本（Git 忽略）
results/quick/          快速数据（Git 忽略）
docs/实验报告.md         正式中文报告
build/                  可执行文件（Git 忽略）
```

## 编译、测试与复现

在 PowerShell 执行，需要 PATH 中的 g++ 和 Python。复用已有作业一绘图环境，不自动安装依赖：

```powershell
Set-Location 'D:/CPP/random/assignment-3'
$env:PYTHONIOENCODING = 'utf-8'
$plotPython = '..\assignment-1\.plot-venv\Scripts\python.exe'
python tools/run.py build
python tools/run.py test
python tools/run.py run --profile quick
& $plotPython tools/analyze.py --results results/quick
& $plotPython tools/analyze.py --results results/quick --audit
& $plotPython tools/plot.py --results results/quick
python tools/run.py run --profile formal
& $plotPython tools/analyze.py
& $plotPython tools/analyze.py --audit
& $plotPython tools/plot.py
& $plotPython tools/report.py
& $plotPython tools/report.py --check
& $plotPython tools/verify.py
```

以上命令会更新相应生成结果，不修改前两次作业。新环境用已装有 numpy、matplotlib、Pillow 的 Python 替代 `$plotPython`；本机版本 2.5.4、3.11.2，字体为 `C:/Windows/Fonts/msyh.ttc`。`run.py all --profile quick` 可合并构建、测试与快速数据生成。审计需要可再生样本文件，样本缺失时先重跑实验；绘图仅需保存的 CSV。

构建参数为 `g++ -std=c++17 -O2 -Wall -Wextra`，完整实际命令、编译器版本、系统、源码与程序哈希保存在 results/environment.json。正式每组分布样本 1000000 个；性能 N=10000、100000、1000000，每设置预热 2 次、测量 20 次，共 300 条。快速每组分布样本 10000 个，性能 N=100、1000，预热 1 次、测量 3 次，共 30 条。

## 随机数与计时约定

所有算法使用 `std::mt19937_64`，显式映射 `(double(rng() >> 12)+0.5)*2^-52` 到开区间 (0,1)。主种子 2026111701，任务顺序种子 2026111702；配置、派生种子与实际命令见 results/run_manifest.json，性能各次实际 seed 见 raw CSV。循环内不重新播种。同工具链与参数的非时间输出可复现，不承诺跨数学库逐位一致。

计时包含随机数、变换、写内存和拒绝采样的所有失败尝试。预分配、初始化引擎、结果完整检查与 checksum、写盘及绘图在计时外。拒绝采样性能版本不维护候选诊断计数，候选与拒绝数只在独立分布实验收集。输出未重新标准化。

## 输出字段与接口

| 文件 | 主要字段与定义 |
|---|---|
| distribution_summary.csv | method,m,n,seed；mean；variance 为 ddof=0；excess_kurtosis 为中心四阶矩/方差平方-3；count_abs_gt2/3、rate_abs_gt2/3；candidates、rejected、acceptance_rate、attempts_per_output |
| theory_reference.csv | N(0,1) 的均值、方差、超额峰度、两侧尾部概率；拒绝法理论接受率和每输出期望候选数 |
| raw/*.csv | method,m,n,repeat（从0开始）,seed（真实uint64十进制）,elapsed_ns（核心纳秒）,checksum（完整输出求和） |
| timing_summary.csv | method,m,n,trials,median_ns,median_ms,ns_per_sample（中位纳秒/N） |
| histogram.csv | method,m,left,right,count,density（count/(N*箱宽)，共用120箱[-6,6]） |
| qq.csv | method,m,rank,p,theoretical,sample；p=(rank-0.5)/N，保留体部和极端尾部 |

非 CLT 方法中的 m=12 仅占位，不影响算法。非拒绝方法的候选与拒绝计数为0，接受率字段为空。所有 CSV 为 UTF-8，Python 标准 csv 格式。

直接调用示例：

```powershell
build/normal.exe --mode distribution --method box_muller --n 3 --m 12 --seed 42 --trials 1 --warmups 0 --output build/three.bin
build/normal.exe --mode performance --method rejection --n 10000 --m 12 --seed 42 --trials 20 --warmups 2 --output build/timing.csv
```

所有选项必填，非法选项非零退出并向 stderr 输出 ERROR。method 为 clt / box_muller / rejection，N 范围1..10000000，m 范围1..1000000，trials范围1..10000，warmups范围0..10000，seed为uint64。分布模式只生成一批，以 seed 直接初始化，输出 N 个本机 double 原始二进制（本环境 little endian、8字节），stdout 为候选计数 JSON。性能模式在计时外从主 seed 初始化的同类引擎派生各 trial seed，输出 CSV。底层变换函数由驱动预先验证参数，测试可注入均匀数序列。

图表、CSV、报告正常纳入 Git；build、缓存、快速结果与百万样本被 assignment-3/.gitignore 排除。无自动提交、推送或根目录忽略规则变更。
