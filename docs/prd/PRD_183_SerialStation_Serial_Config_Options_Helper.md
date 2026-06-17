# PRD-183 Serial Station 串口配置选项 Helper

## 目标

将串口配置工具栏中的波特率、数据位、校验、停止位和流控下拉选项从 `sections.py` 抽出为 `serial_config_options` helper，让串口参数规则集中管理。

## 范围

- 新增 `python/embeddebug/serial_station/ui/serial_config_options.py`。
- 提供 `serial_config_options()`，按字段返回可选值与默认值。
- `sections.py` 构建串口配置下拉时复用该 helper。
- 新增单测覆盖全部默认串口配置选项。

## 非目标

- 不改变串口连接行为。
- 不改变 Profile 读写、连接字段读取或 controller 连接逻辑。
- 不新增串口参数、不改变现有默认值。
- 不新增脚本、打包入口或 legacy native 兼容路径。

## 架构边界

- helper 只属于 UI 层，输出串口配置控件选项与默认文本。
- `sections.py` 继续负责 QWidget 创建、objectName、tooltip、信号装配和默认值写入。
- controller、core、drivers 与 services 不依赖该 helper。

## 三轴状态

| 维度 | 本轮状态 | 说明 |
|---|---|---|
| 工程 | `E4` | 单测、UI 架构测试、UI smoke 与全量 Python 门禁覆盖 |
| 用户 | `U3` | 串口配置入口选项与默认值保持一致，用户路径不变 |
| 设备 | `D2` | 维持 TCP/UDP loopback 与替身验证口径，本轮不声明真实硬件提升 |

## 验收

```powershell
uv run pytest tests\python\unit\test_serial_config_options.py -q
uv run test-embeddebug-py
uv run test-embeddebug-tools
uv run start-embeddebug --smoke
cmd /c EmbedDebug.bat --smoke
```
