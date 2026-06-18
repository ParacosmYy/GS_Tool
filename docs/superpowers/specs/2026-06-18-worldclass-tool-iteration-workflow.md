# 世界级嵌入式调试上位机 — 持续迭代工作流

> 日期：2026-06-18
> 目标：把 EmbedDebug 迭代成世界第一好用的嵌入式调试上位机。
> 策略：每轮 5 个并行 Explore agent 调研 5 个独立功能域 → 产出实现简报 → 单实现 agent 按不重叠边界串行实现+提交 → 循环。

## 一、迭代纪律（遵守 AGENTS.md §10.6）

- **并行只发生在调研/设计阶段**（Explore agent 只读，无冲突）。
- **实现阶段串行**（单一实现 agent，避免多 Agent 改同一文件冲突）。
- 每批功能域必须有**不重叠的文件边界**：各自独立包/目录，不共改 `main.py`/`pyproject.toml`/`uv.lock`（共享入口默认串行收口）。
- 每批收口：`uv run test-embeddebug-py` 全绿 + `--smoke` 双 0 + commit。

## 二、当前能力盘点（已落地）

- 多模式 AppShell（导航栏：串口/OTA/RTT/设置）
- 串口模式：三栏（连接配置/波形+日志+命令/日志工具+Profile）+ 主题 + 图标
- OTA：XMODEM/XMODEM-CRC/YMODEM/YMODEM-g 引擎 + UI 壳（D2 替身验证）
- 波形：基础预览 + 游标 + 图例 + 李萨如/条形图/性能（并行会话推进中）
- 控件库：LED/滑块/按钮/仪表盘/数值显示（并行会话推进中）
- 仪表盘骨架（canvas/factory/palette/tabs）

## 三、下一批 5 个独立功能域（并行调研）

| 域 | 独立边界 | 目标 |
|----|---------|------|
| D1 RTT 实时模式 | `python/embeddebug/rtt/`（新包，不依赖 ui） | 把 RTT 占位变真实：RTT 协议通道 + 实时日志/数据面板 |
| D2 脚本/自动化引擎 | `python/embeddebug/automation/`（新包） | 宏录制/回放 + 触发器（条件→动作）+ 命令队列 |
| D3 数据导出/回放增强 | `python/embeddebug/dataio/`（新包） | CSV/二进制/带时间戳导出 + 回放引擎 + 统计 |
| D4 设置模式真实化 | `python/embeddebug/serial_station/ui/panels/settings_panel.py`（单文件） | 主题切换 + 默认配置 + 快捷键映射 + 关于页 |
| D5 连接侧栏折叠优化 | `python/embeddebug/serial_station/ui/connection_sidebar.py`（单文件） | 端点/高级参数默认折叠（Collapsible），释放左栏空间 |

## 四、循环节奏

1. 派发 5 个 Explore agent 并行调研（各域产出实现简报）。
2. 我按 D1→D5 顺序串行实现（边界不重叠），每完成一个跑测试+提交。
3. 全批收口后，规划下一批 5 个域，循环。

## 五、下一批候选（本批后再定）

- CAN/CAN-FD 总线模式、BLE 调试、Modbus、SPI/I2C 桥接
- 终端分屏/Tab、命令面板增强、响应式断点
- 插件系统、多语言、性能监控面板
