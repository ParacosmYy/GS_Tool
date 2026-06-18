# 批次 2 — 五大调试域 UI 面板接入工作流

> 日期：2026-06-18
> 目标：把已落地但未接入导航的 5 个调试域引擎（RTT/CAN/BLE/Automation + 设置）做成可用 UI 面板，注册进左侧导航栏，让用户能从界面进入每个域。
> 策略：5 个并行 Explore agent 调研各域 UI 面板设计 → 单实现 agent 按不重叠边界串行实现 → 循环。

## 一、当前状态（盘点）

- 引擎骨架已落地（commit 582bdbb1c）：rtt/ can/ ble/ automation/ 各有数据类/引擎，但**全部孤立**，无 UI 面板、无导航注册。
- panels/__init__.py 只注册了 serial/ota/rtt(占位)/settings(占位)。
- 用户打开应用只看到串口/OTA 两个真实面板。

## 二、本批 5 个独立功能域（并行调研）

| 域 | 独立边界 | 现状 | 目标 |
|----|---------|------|------|
| E1 RTT 面板 | `panels/rtt_panel.py`（新单文件）+ panels/__init__ 注册 | rtt/ 引擎孤立，导航是占位 | 真实 RTT 面板：通道选择+文本/数值视图+loopback 验证 |
| E2 CAN 面板 | `panels/can_panel.py`（新单文件）+ 注册 | can/ 引擎孤立，无导航 | CAN 总线面板：帧列表+DBC 解码+发送+统计 |
| E3 BLE 面板 | `panels/ble_panel.py`（新单文件）+ 注册 | ble/ 引擎孤立，无导航 | BLE 面板：设备扫描+GATT 树+读写+日志 |
| E4 Automation 面板 | `panels/automation_panel.py`（新单文件）+ 注册 | automation/ 引擎孤立，无导航 | 自动化面板：规则列表+录制/回放+触发器启用 |
| E5 设置面板 | `panels/settings_panel.py`（新单文件）+ 替换占位 | 占位 | 设置面板：主题切换+快捷键+关于 |

每个域 = 1 个新面板文件 + panels/__init__ 注册行。边界完全不重叠（各面板独立文件，共享入口 panels/__init__.py 串行收口）。

## 三、循环节奏

1. 5 个 Explore agent 并行调研各域面板设计（读引擎 API + OTA 面板范本）。
2. 单实现 agent 按 E1→E5 串行实现（各面板独立文件无冲突），panels/__init__ 串行加注册行。
3. 每个面板补 QSS 覆盖 + 测试，全批收口测试绿 + smoke + commit。
