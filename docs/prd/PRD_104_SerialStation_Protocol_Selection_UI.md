# PRD_104 - Serial Station Protocol Selection UI

## 1. 背景

Serial Station 底层已经具备 `ascii_text`、`modbus_rtu`、`custom_md` 协议注册表和默认协议切换能力，但窗口层仍没有明确的协议选择入口。用户进入上位机后只能看到串口参数和命令发送，无法判断“协议命令”模式到底使用哪一个协议，也无法在不改代码的情况下切换协议。这会让 UART 调试工具看起来像只有裸文本发送，削弱基础上位机可信度。

## 2. 目标

1. 在 Serial Station UI 中新增协议选择面板，展示内置协议列表和当前激活协议。
2. 协议切换必须通过 `SerialStationController` 暴露的 slot 完成，UI 不直接操作协议注册表。
3. 切换协议后，后续 `protocol` 模式发送和接收 dispatcher 使用新的默认协议。
4. 切换成功或失败必须写入结构化系统日志，失败路径不崩溃、不改变当前协议。
5. 新 UI 控件必须具备稳定 `objectName`，用户可见文字使用 `tr()`。
6. 配套 QTest 覆盖协议面板、controller 协议切换和窗口闭环。
7. README 同步更新 Serial Station 对外能力描述，保持企业级宣传入口一致。

## 3. 非目标

- 不新增具体协议实现。
- 不在本轮实现协议参数编辑器。
- 不访问真实 COM 口。
- 不把协议选择持久化到配置文件。
- 不让 UI include `core/` 串口管理器或具体协议目录。

## 4. 架构影响

本轮允许修改：

- `src/apps/serial_station/SerialStationController.*`
- `src/apps/serial_station/SerialStationWindow.*`
- `src/apps/serial_station/ui/`
- `tests/serial_station/`
- `CMakeLists.txt`
- `README.md`

边界约束：

- `SerialStationWindow` 只做装配和 signal/slot 编排。
- `SerialProtocolPanel` 只展示协议列表、当前状态和发出选择信号。
- `SerialStationController` 负责校验协议名、更新 registry 默认协议并重置 dispatcher。
- `core/` 不 include 具体协议目录。
- `protocols/` 不 include UI、QWidget 或 controller。

## 5. 验收标准

- 打开 Serial Station 后能看到协议选择控件，默认显示 `ascii_text`。
- 协议列表包含 `ascii_text`、`modbus_rtu`、`custom_md`。
- 选择 `custom_md` 后 controller 默认协议变为 `custom_md`，系统日志记录切换成功。
- 选择不存在的协议时记录错误，不改变当前默认协议。
- `SerialCommandPanel` 的 `protocol` 发送路径继续复用 controller 的默认协议。
- 新增源码已加入 CMake。
- `test_serial_protocol_panel`、`test_serial_station_controller`、`test_serial_station_workbench` 通过。
- 全量 Serial Station QTest 通过。
- `EmbedDebug.bat` 探针仍通过。

## 6. 失败条件

- UI 直接 include 具体协议实现或 `core/SerialManager.h`。
- 切换协议只改变下拉框，不影响 controller 默认协议。
- 无效协议导致崩溃或清空当前协议。
- 新增面板没有 objectName 或出现未包裹 `tr()` 的用户可见字符串。
- README 未同步对外能力说明。
