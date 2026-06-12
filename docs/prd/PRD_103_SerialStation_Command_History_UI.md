# PRD_103 - Serial Station Command History UI

## 1. 背景

Serial Station 已具备 UART 配置、命令发送、日志、导出和回放预览，但命令面板仍缺少“最近命令”入口。实际调试 MCU UART 时，用户经常重复发送 `AT+GMR`、`PING`、寄存器读取或 HEX 帧。当前只能重新输入或点少量固定快捷按钮，基础上位机体验不完整。

## 2. 目标

1. 在 `SerialCommandPanel` 内增加最近命令历史。
2. 最近命令按最新优先展示，去重后保留有限条数。
3. 发送成功触发 UI 历史记录更新；空命令不进入历史。
4. 用户能从历史下拉/列表选择命令并回填输入框。
5. 提供清空历史入口，清空后保持发送输入可用。
6. 历史逻辑只属于 UI 层，不拼接协议帧，不调用 `SerialManager`。
7. 配套 QTest 覆盖去重、容量、选择回填、清空和 send signal 不回归。

## 3. 非目标

- 不做持久化配置。
- 不新增协议命令语义。
- 不改变 `SerialStationController::sendCommand` 行为。
- 不访问真实 COM 口。
- 不修改 `core/`、`protocols/`、`services/` 边界。

## 4. 架构影响

本轮改动属于 Serial Station `ui/` 层：

- 允许修改 `src/apps/serial_station/ui/SerialCommandPanel.*`。
- 允许新增 UI 层轻量历史模型，用于管理字符串列表。
- 允许同步 `CMakeLists.txt` 与 `tests/CMakeLists.txt` 注册新增 `.h/.cpp`。
- 禁止 UI include `core/SerialManager.h`、具体协议目录或服务层导出类。

## 5. 验收标准

- 最近命令控件有稳定 `objectName`，用户可见文字使用 `tr()`。
- 发送 `PING` 后，历史中出现 `PING`。
- 连续发送重复命令时，历史只保留一条并移到最前。
- 超出容量时丢弃最旧项。
- 点击历史项会回填输入框并刷新发送按钮状态。
- 清空历史后历史控件变空且不触发发送。
- `test_serial_command_panel` 和 `test_serial_station_workbench` 通过。
- 全量 Serial Station QTest 通过。
- `EmbedDebug.bat` 探针仍通过。

## 6. 失败条件

- 历史逻辑写入 controller/core/protocols。
- UI 槽函数拼接 `QByteArray` 协议帧。
- 新增源码未加入 CMake。
- 历史命令包含空字符串或重复项。
- 清空历史导致当前输入框不可编辑。
