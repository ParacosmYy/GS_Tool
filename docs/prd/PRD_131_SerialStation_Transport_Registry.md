# PRD-131 Serial Station Transport Registry

## 目标

把 Serial Station 连接驱动从 controller 构造细节中拆出，形成可注册、可查询、可替换的 transport registry。当前只承接 fake 与 serial 两类驱动，后续 TCP、UDP、BLE、CAN、RTT 等接入不得继续把具体驱动写进 controller。

## 范围

- 新增 `python/embeddebug/serial_station/drivers/registry.py`。
- `SerialWorkbenchController` 通过 registry 查询端口、创建 transport。
- 保持现有 UI 行为、profile 字段和 fake/serial 用户路径不变。
- 不新增 UI 控件，不改变启动入口，不引入新依赖。

## 架构约束

- registry 属于 `drivers/`，只负责驱动注册、创建和端口枚举。
- controller 只按 mode 请求 transport，不直接依赖具体驱动类。
- transport 仍遵循 `SerialTransport` 契约。
- 本轮不实现 TCP/UDP，只为后续多源接入提供扩展点。

## 验收

- registry 可列出默认 mode：`fake`、`serial`。
- registry 可为 fake 创建内存 transport。
- registry 可替换 serial factory 与端口 provider，controller 仍能完成 serial connect、send、profile。
- `uv run test-embeddebug-py` 通过。
- `uv run start-embeddebug --smoke` 与 `cmd /c EmbedDebug.bat --smoke` 通过。

## 状态口径

- 工程状态：E4，自动化测试覆盖 registry 与 controller 既有路径。
- 用户状态：U3，现有 fake/serial 主流程不回退。
- 设备验证：D1，本轮只做单测与 smoke，不宣称真实硬件提升。
