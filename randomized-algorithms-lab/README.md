# 随机化算法课程仓库

记录随机化算法课程的编程作业、实验代码、测试数据和实验报告。

## 已有内容

| 目录 | 主题 | 内容 |
|---|---|---|
| [assignment-1](assignment-1/README.md) | Freivalds 矩阵乘法随机验证 | 精确验证与随机验证实现、正确性检查、性能与漏检实验、图表和报告初稿 |
| [assignment-2](assignment-2/README.md) | 抽样区间宽度与随机化中位数 | 两次排序的抽样算法、失败重试、默认正式实验、新种子确认、图表和中文报告 |
| [assignment-3](assignment-3/README.md) | 正态随机变量的生成 | CLT、Box–Muller、指数拒绝采样，百万样本分布、性能实验、图表与中文报告 |
| [assignment-4](assignment-4/README.md) | 蒙特卡洛积分 | 普通均值、等宽分层、精度与耗时、独立复验及 PDF 报告 |

## 作业一：Freivalds 矩阵乘法随机验证

给定整数方阵 A、B、C，判断 AB=C。比较 O(n³) 的精确验证与 O(kn²) 的 Freivalds 随机验证，分析运行时间和漏检概率随规模、验证轮数的变化。

已完成两种算法实现、测试数据与独立真值检查、正式性能和漏检实验，并生成 8 张图表及实验报告初稿。

- [运行说明与实验结果](assignment-1/README.md)
- [算法源码](assignment-1/src/solver.cpp)
- [实验报告](assignment-1/docs/实验报告.md)
- [规模与运行时间](assignment-1/results/figures/n_time.png)
- [加速比](assignment-1/results/figures/speedup.png)
- [实测漏检率与理论概率](assignment-1/results/figures/probability.png)

代码使用 C++17，数据检查与实验调度使用 Python，绘图使用 matplotlib。编译、复现实验和依赖说明见作业目录内的 README。

## 作业二：抽样区间宽度与随机化中位数

固定样本量和候选集合上限，比较五种抽样排名区间参数的成功率、候选大小、单次尝试和含失败重试的完整求解耗时。默认性能实验运行到一百万元素，并用新数据与新采样种子确认接近参数的表现。

- [运行说明与字段定义](assignment-2/README.md)
- [算法实现](assignment-2/src/median.hpp)
- [中文实验报告](assignment-2/docs/实验报告.md)
- [区间参数与完整求解均值](assignment-2/results/figures/alpha_solve_mean.png)
- [单次失败率](assignment-2/results/figures/alpha_failure_rate.png)

## 作业三：正态随机变量的生成

自行实现中心极限定理近似法、Box–Muller 变换和指数提议的拒绝采样，统一输出标准正态目标的 double 样本。比较 CLT 的 m=3、12、48 与另外两种方法，共五种设置；不使用 std::normal_distribution，也不按实测均值、方差重新标准化。

已完成 C++17 实现、编译、确定性测试、快速检查、正式分布与性能实验，并生成中文报告和三张结果图。23,558 项 C++ 检查和 14 类 CLI 非法参数检查通过；正式分布每组生成 1,000,000 个输出，性能在 N=10,000、100,000、1,000,000 下，每设置预热 2 次、测量 20 次，共保留 300 条原始计时。

- [运行说明、复现命令与字段定义](assignment-3/README.md)
- [三种生成算法](assignment-3/src/normal.hpp)
- [实验驱动与核心计时](assignment-3/src/driver.cpp)
- [正确性与分支测试](assignment-3/tests/test_normal.cpp)
- [中文实验报告](assignment-3/docs/实验报告.md)
- [分布统计汇总](assignment-3/results/distribution_summary.csv)
- [性能中位数汇总](assignment-3/results/timing_summary.csv)
- [密度直方图](assignment-3/results/figures/density.png)
- [正态 Q–Q 图](assignment-3/results/figures/qq.png)
- [生成速度对比](assignment-3/results/figures/speed.png)
- [交付一致性检查](assignment-3/results/delivery_checks.json)

百万输出的中位耗时分别为：CLT m=3 为 16.282 ms、m=12 为 82.699 ms、m=48 为 240.477 ms，Box–Muller 为 30.726 ms，指数拒绝采样为 50.508 ms。拒绝采样的实测接受率为 76.0455%，理论值约为 76.0173%。这些速度为本机本轮实测；CLT m=3 的支持范围为 [-3,3]，没有绝对值大于 3 的输出，报告结合尾部统计与 Q–Q 图解释其近似误差。

本机精简提交副本位于 `D:/CPP/随机化1117`，参照 `随机化1020`、`随机化1103` 整理。13 个文件全部平铺，包含源码、测试、正式配置、三份汇总 CSV、三张 PNG、Markdown 报告、7 页 PDF 报告及文件说明；PDF 路径为 `D:/CPP/随机化1117/实验报告.pdf`。该目录位于仓库外，仓库内保留完整实验工程与 Markdown 报告。

三种算法使用相同的 mt19937_64 引擎与开区间 (0,1) 均匀数映射，种子、真实编译命令、环境及哈希均已记录。可再生的百万样本、构建产物和快速结果被 Git 忽略，正式汇总、原始计时、图表及报告随仓库保存。

## 作业四：蒙特卡洛积分

估计 exp(−x²) 在四种范围内的积分，并在固定 [0,4] 上比较四种等宽分层。正式实验共测量 28 个配置、1,400 次估计，另使用新种子复验两种候选配置。

- [运行说明与复现命令](assignment-4/README.md)
- [算法实现](assignment-4/src/integral.hpp)
- [实验报告](assignment-4/docs/实验报告.md) · [PDF](assignment-4/docs/实验报告.pdf)
- [主实验汇总](assignment-4/results/main_summary.csv) · [独立复验](assignment-4/results/confirmation_summary.csv)
- [范围与相对 RMSE](assignment-4/results/figures/range_rmse.png) · [分层与相对 RMSE](assignment-4/results/figures/strata_rmse.png) · [精度与耗时](assignment-4/results/figures/accuracy_time.png)
