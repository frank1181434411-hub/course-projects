# 实验二：网络编程基础

实验日期：2026-11-28（Asia/Shanghai）。状态：**已构建、运行并验证**。姓名、学号和班级留空。

## 实验目的

理解 Windows 网络编程和套接字；掌握 TCP 服务端/客户端的生命周期；实现双方均能主动发送、独立接收的全双工控制台通信。

## 实验原理

Winsock 通过 `WSAStartup(MAKEWORD(2,2))` 初始化。服务端依次 socket、bind、listen、accept；客户端 socket、connect。TCP 提供可靠字节流，但单次 recv 不对应一次 send，因此应用采用换行分帧，并循环处理部分发送、拆分或合并的消息。双方分别运行接收线程和输入处理，使任一方均可主动发送。

## 实验设计过程

使用 MSVC x64、C++17、Windows SDK 和 CMake，链接 Ws2_32.lib。入口是 src/server.cpp 和 src/client.cpp，构建目标 lab02_server、lab02_client；通用 RAII 和通信逻辑在 ../common/。服务端默认仅监听 127.0.0.1:9325，支持参数端口。

接收线程阻塞 recv，设置短超时以检查结束标志；主线程轮询控制台事件或重定向的管道输入。回车发送消息，`/quit` 表示主动结束。接收 0 字节视为对端正常关闭。退出先 shutdown，等待接收线程 join，再由 RAII closesocket 和 WSACleanup 释放资源；没有 detach 后继续使用已销毁对象的问题。消息缓冲区设置上限，错误通过 WSA 错误码报告。

## 实验实现及结果分析

在项目根目录执行 `cmake --preset msvc-x64`、`cmake --build --preset debug`，Debug x64 构建成功。环境为 MSVC cl 19.52.37029.0、Windows SDK 10.0.26100.9457，程序无需外部 C++ 库。

运行两个终端的方法：

```powershell
# 终端 A
.\out\build\msvc-x64\Debug\lab02_server.exe 9325
# 终端 B
.\out\build\msvc-x64\Debug\lab02_client.exe 127.0.0.1 9325
```

实测使用 `tests/run_console_demos.py` 打开两个可见 Windows 控制台，分别驱动真实服务端和客户端。定时输入由脚本完成，非人工键入。端口 19425，双方是独立进程，实际输出如下（完整精简记录在 evidence/）。

```text
服务端 SENT: server-first-in-console
客户端 RECV: server-first-in-console
客户端 SENT: client-first-in-console
服务端 RECV: client-first-in-console
服务端 SENT: server-second-in-console
客户端 RECV: server-second-in-console
客户端 SENT: client-second-in-console
服务端 RECV: client-second-in-console
客户端 LOCAL QUIT
双方 SESSION CLOSED，退出码均为 0
```

额外通过 verify_tcp.py 实测：服务端先发消息、双方独立发送、UTF-8 与 9000 字节消息、TCP 写入拆分/合并后分帧、客户端 /quit、服务端 /quit 和远端 EOF。测试均通过。端口使用 19325/19335/19345，与普通实验端口分开。这证明通信不依赖严格“一问一答”，并且断开不要求对端继续键盘输入才能退出。

## 问题及解决方案、心得体会

课程样例对 recv 返回 0 的处理容易造成循环空读，且 detach 会使清理时序难以控制。本实现将 0 明确作为断开，并 join 接收线程。单次 send/recv 的边界不能作为消息边界，拆包与合包测试验证了缓冲分帧设计。通过对比服务端先发和客户端先发的记录，可以区分全双工能力与简单回显。

限制：单服务端单连接、本机 IPv4 实验，不包含多客户会话、加密认证或消息历史持久化。
