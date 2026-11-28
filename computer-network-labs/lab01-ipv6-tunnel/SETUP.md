# 实验一：按指导书图 1 操作（待 Packet Tracer 实测）

依据指导书正文第 1—2 页，而非 PPT 的简化拓扑。指导书没有指定 IP 地址；下面是本项目自行规划的文档地址。此文件是待执行方案，配置尚未在 Packet Tracer 验证。不要将附带样例重命名为自己的成果。

## 1. 安装并放置设备

从 [Cisco 官方资源页](https://www.netacad.com/resources/lab-downloads) 下载当前稳定版 Windows 64 位 Packet Tracer。安装目录优先 D 盘。首次启动在官方窗口登录；不将账号口令提供给脚本。

在 Logical 工作区放置两台 **2811**（命名 R1/R2）、两台 **2960-24TT**（SW0/SW1）和四台 PC（PC0—PC3）。图 1 左右各两台 PC，PC1 与 PC2 为跨网段测试端。

| 端点 A | 端点 B | 连接线 |
|---|---|---|
| R1 FastEthernet0/0 | R2 FastEthernet0/0 | Copper Cross-Over |
| R1 FastEthernet0/1 | SW0 FastEthernet0/1 | Copper Straight-Through |
| R2 FastEthernet0/1 | SW1 FastEthernet0/1 | Copper Straight-Through |
| PC0 FastEthernet0 | SW0 FastEthernet0/2 | Copper Straight-Through |
| PC1 FastEthernet0 | SW0 FastEthernet0/3 | Copper Straight-Through |
| PC2 FastEthernet0 | SW1 FastEthernet0/2 | Copper Straight-Through |
| PC3 FastEthernet0 | SW1 FastEthernet0/3 | Copper Straight-Through |

若新版没有该型号，选择支持 `tunnel mode ipv6ip` 的路由器，先运行 `show ip interface brief`，依据实际接口名称修改两份配置和接线表，不能照搬接口名。

## 2. 路由器、交换机、PC 配置

打开路由器 CLI，对初始化向导回答 `no`，回车进入提示符，按顺序粘贴对应的 `router-config/R1.cfg`、`R2.cfg`。最后 `copy` 的目标文件名提示回车确认。配置涉及的地址如下。

| 设备 | IPv6 地址 /64 | 默认网关 |
|---|---|---|
| PC0 | 2001:db8:1::10 | 2001:db8:1::1 |
| PC1 | 2001:db8:1::11 | 2001:db8:1::1 |
| PC2 | 2001:db8:2::12 | 2001:db8:2::1 |
| PC3 | 2001:db8:2::13 | 2001:db8:2::1 |

PC → Desktop → IP Configuration → IPv6 Configuration，填静态地址、前缀长度 64 和网关。SW0/SW1 是二层接入交换机，无需配置管理 IP。CLI 分别 `enable`、`configure terminal`、`hostname SW0`（另一台 SW1）、`end`、`copy running-config startup-config`；接入端口使用默认 VLAN 1，保持启用并等待 STP 收敛。

## 3. Realtime 连通性验收

R1 依次运行：

```text
show ip interface brief
show ipv6 interface brief
ping 10.1.1.2
show interfaces tunnel 0
ping 2001:db8:12::2
show ipv6 route
```

R2 做反向测试（IPv4 对端 10.1.1.1，隧道对端 2001:db8:12::1）。应确认物理接口及 Tunnel0 为 up/up，路由表有对侧 LAN 的静态路由。然后 PC1 → Desktop → Command Prompt：`ping 2001:db8:2::12`，PC2 反向 `ping 2001:db8:1::11`。再测试 PC0 ↔ PC3。首次邻居发现可能导致首包超时，等待收敛后重复并记录实际成功数，不把预期写成结果。

## 4. Simulation 封装观察

右下角切换 **Simulation**，Edit Filters 中开启 ICMP、ICMPv6（版本支持时也开启 IPv4/IPv6）；清空旧事件。由 PC1 ping PC2，用 Capture/Forward 逐事件查看 SW0、R1、R2、SW1 的 PDU Details/OSI Model，分别查看 Inbound/Outbound。

预期：SW0 转发 Ethernet / IPv6 / ICMPv6；R1 的 LAN 入站是原生 IPv6；R1 的 WAN 出站和 R2 的 WAN 入站增加外层 IPv4，源 10.1.1.1、目的 10.1.1.2、Protocol=41，内部 IPv6 源/目的保持 PC1/PC2；R2 解封装后经 SW1 转发原生 IPv6。回包方向相反。二层 MAC 每段变化，隧道是 IPv6-over-IPv4，不是 NAT 或应用层代理。

这些均为理论预期，尚未观察。实际 UI 若未展开协议 41，记录实际可见字段与版本，不编造截图。

## 5. 保存自己的成果与验证样例兼容性

File → Save As → 本目录 `topology.pkt`。通过 CLI `show running-config` 导出实际 R1/R2 配置，与当前方案核对。将 ping 成功数、Tunnel0 状态、Simulation 字段和截图补充到 report.md。当前仓库没有占位或伪造的 topology.pkt。

另用 File → Open 打开课程目录中的 `实验一资料/实验样例（拓扑和配置命令与实验内容有差别）.pkt`，保留原件，记录打开成功与否。失败时记录完整报错文本；新版可按上面步骤独立重建图 1，不依赖旧样例。当前尚未试开样例。
