# PRD-134 Serial Station TCP UI Entry

## 目标

把已验证的 TCP client transport 接入 Serial Station 连接工具栏，让用户能在 PyQt 工作台直接填写 host、port 并建立 TCP 字节通道。

## 范围

- 在连接工具栏增加 TCP host、TCP port 和 Connect TCP 控件。
- UI 仅收集用户输入并调用 controller，不处理 socket 细节。
- 成功连接后同步状态栏、连接按钮状态和 profile 保存内容。
- UI smoke 覆盖 TCP endpoint 连接与 profile 持久化。

## 非目标

- 不新增 UDP、多会话标签或网络扫描。
- 不改变协议解析、日志、回放和测量服务。
- 不把 TCP 作为真实设备完成状态宣传。

## 验收

- TCP host、port、连接按钮均有稳定 objectName。
- 点击 Connect TCP 后调用 controller 的 TCP 连接入口。
- 成功连接后状态栏显示 TCP endpoint，连接按钮进入已连接状态。
- 保存 profile 时记录 `tcp` mode 与 `host:port` endpoint。
- `uv run test-embeddebug-py` 与启动 smoke 通过。

## 三轴状态

- 工程状态：`E4`，UI smoke 覆盖用户入口和 profile 闭环。
- 用户状态：`U3`，用户可在主工作台直接建立 TCP client 连接。
- 设备状态：`D2`，已有本机 loopback 替身验证，真实外设验证仍待补证。
