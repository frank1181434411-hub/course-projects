# Course Projects

本仓库汇总了本科阶段完成的部分课程设计与综合实验项目，内容涵盖游戏开发、计算机图形学、桌面应用、数据库系统以及数据结构与算法。

各项目均保留独立的源代码、项目说明和构建方式。有关功能实现、开发环境及运行方法，请进入相应子目录查看项目 README。

## 项目一览

| 项目 | 课程 / 方向 | 技术栈 | 简介 |
| --- | --- | --- | --- |
| [Tank Battle](./tank-battle/) | 游戏程序设计 | C++17、SFML 3.1、CMake | 复古坦克大战游戏，包含地形碰撞、敌方 AI、道具、局域网双人合作、菜单与 HUD 等功能。 |
| [Graphics Algorithm Painter](./graphics-algorithm-painter/) | 计算机图形学 | C++、Qt Widgets、qmake | 图形学算法交互程序，实现折线路径道路生成、点与多边形位置判定，以及 Hermite、Bezier、B-Spline 曲线绘制。 |
| [Campus Lab Manager](./campus-lab-manager/) | 数据库原理与应用 | Python 3.12、Flask、MySQL、PyMySQL | 高校机房管理系统，覆盖用户权限、机房与机位、排课考勤、上下机记录和设备报修等业务。 |
| [Simple Paint](./simple-paint/) | C++ / Qt 综合实践 | C++23、Qt Widgets、qmake | 简易桌面画板，支持基本图形绘制、样式设置、图形选择与多选、删除及绘图文件读写。 |
| [Traffic Query System](./traffic-query-system/) | 数据结构与算法综合实验 | C++、Qt、图、线段树 | 交通路线查询系统，支持按距离、费用和时间规划路线，并使用线段树维护 Dijkstra 算法中的节点距离。 |

## 项目说明

### Tank Battle

基于 SFML 开发的坦克对战游戏。项目实现了玩家移动与射击、敌方坦克、砖墙破坏、基地保护、随机道具、三档 AI 难度以及局域网联机等功能，并配有较完整的需求分析、总体设计、测试记录和答辩文档。

[查看项目详情](./tank-battle/)

### Graphics Algorithm Painter

在 Simple Paint 的窗口、画布与图形对象框架基础上扩展而成的计算机图形学实验程序。项目重点展示道路边界生成、射线法点内外判定、参数曲线生成以及控制点交互编辑。

[查看项目详情](./graphics-algorithm-painter/)

### Campus Lab Manager

面向高校公共机房和教学实验室的数据库应用系统。系统采用统一账号与角色权限设计，将学生上机、课堂考勤、机房排课、机位状态和设备维修等业务纳入同一数据模型，并通过事务、约束和参数化 SQL 维护数据一致性。

[查看项目详情](./campus-lab-manager/)

### Simple Paint

基于 Qt Widgets 的桌面绘图程序。项目采用面向对象方式设计图形基类与派生类，支持直线、矩形、椭圆和菱形绘制，并实现命中检测、Ctrl 多选、删除、颜色与线宽设置及文件保存和读取。

[查看项目详情](./simple-paint/)

### Traffic Query System

面向交通网络的图算法综合实验。系统将城市建模为节点、交通线路建模为有向边，可分别查询最短距离、最低费用和最短时间路线，同时支持路径回溯、图形界面输入以及本地数据持久化。

[查看项目详情](./traffic-query-system/)

## 仓库结构

```text
course-projects/
├── README.md
├── tank-battle/
├── graphics-algorithm-painter/
├── campus-lab-manager/
├── simple-paint/
└── traffic-query-system/
```

## 使用说明

本仓库中的项目彼此独立，所需的编译工具、运行环境和依赖并不完全相同。请进入目标项目目录，按照其中的 README 完成配置、构建和运行。

部分项目为团队课程设计；具体成员、分工及实现说明以对应项目中的文档为准。

## 说明

本仓库主要用于课程成果整理、学习记录与项目展示。项目仍可能存在未完善功能或仅适用于原开发环境的配置，具体状态请以各项目 README 为准。
