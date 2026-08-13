# ADR 0008：M6 使用 attach-only RTT Telnet bridge

## 背景

J-Link RTT 需要目标 RAM 中的 RTT 控制块和一个活动的 J-Link debug session。当前用户环境
尚未安装 J-Link 驱动/工具，且首版目标是最省事的 Windows EXE。直接绑定 SEGGER DLL、启动
J-Link Commander 或托管 `JLinkRTTClient.exe` 会引入供应商授权、进程生命周期、探针选择和
调试命令边界，不能在没有硬件的环境里诚实验收。

## 决策

- `RttTransportConfig` 使用显式 `PeerAddress`、有限 connect/read/write timeout、bounded
  read chunk、schema v1 和 channel 0/1；默认 peer 是 `127.0.0.1:19021`。
- `RttTransport` 连接已经运行的 RTT Telnet 服务，不启动外部进程，不加载 SDK/DLL，不访问
  probe、memory、halt、run、reset 或 flash。
- 连接建立后，在 SEGGER 规定的短配置窗口发送
  `$$SEGGER_TELNET_ConfigStr=RTTCh;<channel>$$`，随后将通道视作一个有界 byte stream。
- 复用现有 `SessionManager`、`StreamTransportPort`、`StreamChunk`、`StreamWrite`、原始
  recorder、终端和发送历史；Down 命令使用 `RttDownWrite(channel, StreamWrite)`，endpoint
  metadata 同时保留 bridge、peer 和 channel。
- RTT 不接入 M4a/M4b parser/component bridge，首版只走 raw terminal/raw record。

## 取舍与风险

标准库 TCP bridge 最小且可替换，避免把普通桌面工具耦合到供应商 ABI；代价是用户必须
先启动提供 RTT Telnet 的 J-Link 工具，且连接失败只能准确报告为 socket/open/EOF 等
传输事实，不能推断具体是探针占用还是目标未初始化。多个 Telnet 客户端、Down buffer
消费/丢失、断线吞吐和目标兼容性必须用授权硬件单独验收。

## 验收边界

已通过本地标准库回环验证 channel 1 Config String、Up bytes、Down bytes 和关闭路径，
并沿用 Ruff、compileall、offscreen Qt 和 PyInstaller 启动门。未宣称 SEGGER 工具、驱动、
目标板或真实 RTT 硬件通过；真实硬件安装和授权由用户环境负责。
