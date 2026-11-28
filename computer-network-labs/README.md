# 计算机网络实验一、二、三

本项目按《计算机网络-实验指导书(2025-2026-1)》实现。课程原件、旧安装包、含个人信息的报告样例和 FTP 凭据不进入仓库。实验日期：2026-11-28（Asia/Shanghai）。

| 实验 | 入口 | 实际状态 |
|---|---|---|
| 一：网络设备认知与 IPv6 隧道 | [接线、地址及操作步骤](lab01-ipv6-tunnel/SETUP.md)、[路由器配置](lab01-ipv6-tunnel/router-config/)、[报告](lab01-ipv6-tunnel/report.md) | 方案已整理；Packet Tracer 未安装/登录，拓扑、ping、Simulation、旧样例兼容性均待实测；尚无 topology.pkt |
| 二：Winsock 全双工 TCP | lab02_server、lab02_client；[报告](lab02-winsock-basics/report.md) | MSVC x64 编译、两控制台双向消息和退出验证通过 |
| 三：进程通信和图形 FTP | lab03_server、lab03_client、lab03_ftp；[报告](lab03-windows-socket/report.md) | 独立进程 TCP、Win32 FTP 登录/列目录/错误提示/界面响应验证通过 |

## 环境与构建

Windows 11；Visual Studio **2026 Build Tools** 的“使用 C++ 的桌面开发”、MSVC x64/x86 与 Windows SDK；CMake ≥3.25 且支持 Visual Studio 18 2026 生成器（本机实测 4.3.1-msvc1 和 PATH 上的 4.3.4）。C++17，仅使用标准库、Winsock 与 Win32 API，不需要 Qt 或第三方 C++ 库。

本机复用 MSVC 工具目录 14.52.37025（cl 19.52.37029.0）、Windows SDK 10.0.26100.9457（目录版本 10.0.26100.0）、VS Code 1.147.0、C/C++ 1.35.6 和 CMake Tools 1.25.10。Build Tools 在 D:\NetworkLabs\tools\VSBuildTools；SDK 保留在 C:\Program Files (x86)\Windows Kits\10。共享配置不包含这些本机绝对路径。

在本目录运行 PowerShell：

```powershell
cmake --preset msvc-x64
cmake --build --preset debug
# 可选发布构建：cmake --build --preset release
```

如果 `cmake` 不在 PATH，可从 VS 的 Developer Command Prompt 执行；本机还可直接使用 Build Tools 自带 CMake 的完整路径。所有构建输出均在 `out/build/msvc-x64/`，Git 忽略整个 `out/`。

## VS Code 工作区

打开本目录 `computer-network-labs.code-workspace`，资源管理器中可见三个 lab 目录。接受推荐的 Microsoft C/C++ 与 CMake Tools。选择 `msvc-x64` 配置预设，`Ctrl+Shift+B` 执行配置及 Debug 构建；“运行和调试”中有五个对应入口。TCP 调试先启动服务端，再用新调试会话启动客户端。两个 TCP 会话各有独立外部终端；FTP 入口打开 Win32 窗口。编译信息由 CMake Tools 提供给 C/C++ 扩展。

任务、调试路径使用工作区相对引用；不改全局 VS Code 设置。需要其他本机配置时放 `.local/` 或 CMakeUserPresets.json，不提交。

已在此工作区实际按 F5 验证 Lab 02 TCP Server：前置 CMake 配置/构建任务完成，cppvsdbg 启动真实服务端并加载 Ws2_32.dll，随后停止调试。其余四项调试入口已按相同路径配置；实际程序运行验证详见各实验报告。

## 实验二运行

分别在两个终端中、从本目录运行：

```powershell
# 终端 A
.\out\build\msvc-x64\Debug\lab02_server.exe 9325
# 终端 B
.\out\build\msvc-x64\Debug\lab02_client.exe 127.0.0.1 9325
```

双方均可主动输入消息，回车发送；`/quit` 主动退出，或者关闭标准输入。服务端仅绑定 127.0.0.1，单次接受一个连接，退出后重新启动可继续实验。采用换行分帧、send 循环和 recv 缓冲，支持跨缓冲区消息。接收线程与输入处理独立运行，结束时 join 线程并释放 socket/WSA。

## 实验三运行

```powershell
# 终端 A / B 分别运行
.\out\build\msvc-x64\Debug\lab03_server.exe 9326
.\out\build\msvc-x64\Debug\lab03_client.exe 127.0.0.1 9326
# 图形 FTP 客户端
.\out\build\msvc-x64\Debug\lab03_ftp.exe
```

FTP 窗口输入 IPv4、端口、专用账号和密码，点击“连接并列目录”或“重新连接 / 刷新”。程序发送 USER、PASS、CWD /、TYPE A、EPSV（不支持时回退 PASV）、LIST、QUIT。数据连接只连接输入的服务器，避免信任 PASV 返回的其他地址；接收并解析多行响应，列表和控制响应分开显示。每次刷新新建连接，完成后正常关闭。

本机复用 pyftpdlib 2.2.0，监听 127.0.0.1:2121，根目录 D:\NetworkLabs\ftp-root，测试文件 networklabs-test.txt。启动方式为本机 `D:\NetworkLabs\start-ftp.cmd`；凭据由本机 `show-ftp-credentials.cmd` 查看，不能写入仓库或命令行。其他机器可自行配置普通 FTP 本机服务器，修改界面输入即可。

密码框掩码显示，PASS 参数不进入日志，服务器响应/目录列表也做密码脱敏。网络连接、认证和目录读取位于后台线程，UI 通过 PostMessage 接收结果。连接超时 3 秒、控制通道读写超时 5 秒，并设总操作时间检查；关闭窗口时如有网络操作则提示等待结束，窗口消息循环保持响应。此客户端演示普通 FTP，未实现 FTPS、文件上传下载、持久会话和任意 FTP 命令输入框。

## 可复现验证

以下脚本调用真实编译产物，不模拟成功结果。仅需 Python 3；GUI 测试需本机 FTP 凭据辅助模块，密码从当前用户的 DPAPI 文件读取，脚本不打印密码。GUI 测试操作真实 Win32 控件，验证错误场景，并截取客户端自身窗口。

```powershell
python tests/verify_tcp.py
python tests/run_console_demos.py
# 本机专用（服务器需已启动）：
python tests/verify_ftp_gui.py --credentials-helper D:\NetworkLabs\tools\ftp_lab.py
```

`run_console_demos.py` 打开两组可见 Windows 控制台，各控制台驱动一个真实 TCP 程序并显示其输出，定时输入测试消息，最后自动退出。普通交互实验直接运行上面的 exe 命令。

原始复验日志写入 `out/verification/`；精简脱敏记录和 FTP 实测截图见 [evidence/](evidence/)。实验报告不将脚本驱动的输入描述成人工键入。

## 报告与尚待完成项

三个报告 Markdown 版本随代码提交。正式 Word 版按对应课程模板的版式及章节生成，保留在本机 D:\NetworkLabs\reports；封面个人信息留空，避免套用样例中他人的信息。实验一正式文件带“待验证”标记，补完 Packet Tracer 实操后才能作为完整报告提交。

实验一需要：从 Cisco 官方资源页登录下载稳定版；实际搭建图 1 的八台设备；粘贴并校核接口配置；PC1↔PC2 及 PC0↔PC3 ping；Simulation 记录协议 41 封装；另行试开课程旧样例；保存自己的 topology.pkt 和实际 running-config。没有用旧 `.pkt` 冒充成果。

## 参考

- 课程指导书正文第 1—5 页及实验一 PPT；章节结构遵照对应报告模板。
- [Microsoft Winsock 示例](https://learn.microsoft.com/en-us/windows/win32/winsock/complete-server-code)
- [Win32 PostMessage](https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-postmessagew)
- [RFC 959：FTP](https://www.rfc-editor.org/rfc/rfc959)、[RFC 2428：EPSV](https://www.rfc-editor.org/rfc/rfc2428)
