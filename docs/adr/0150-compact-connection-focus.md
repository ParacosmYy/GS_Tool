# ADR-0150：紧凑窗口自动保护连接配置首屏

日期：2026-08-12

状态：已采用（ARCH-99 / UI-1.172）

## 背景

在 980×720 总览模式下，连接配置页与实时观测、终端、发送区共享根窗口垂直空间，稳定布局中
`workspaceTabs` 约 115px，连接页只能显示顶部；下方三个 surface 仍占约 96/162/98px。虽然
用户可以手动点击“专注设置”，但第一次进入紧凑窗口时配置表单已经被挤压，和配置页应提供完整
首屏的交互预期不符。

## 决策

`workspace_runtime.py` 以 `760px` 为紧凑高度上限，连接页在未被用户显式覆盖时自动复用既有 focus
过渡。窗口 resize 和 Tab 切换调用同一个同步入口；focus override 仅记录连接页上用户的明确选择，
不复制 SessionState 或配置值。`workspace_focus_transition.py`、唯一 `MotionController`、既有
outer `QScrollArea` 和下方 surface owner 保持不变。CTA 进入连接页时先写入 focus intent，避免两段
相反动画竞争。

## 验证与边界

离屏组合根验证覆盖 980×720 新窗口、1240×820 宽屏、宽屏缩到紧凑、显式返回总览、Tab 往返、
连接 CTA、三主题和 UART/TCP Client/TCP Server/UDP/BLE/RTT 六种传输；紧凑连接页 viewport 为
约 471px，横向滚动最大值均为 `0`。Ruff、compileall、source-limit、theme token audit 和
provenance verify 通过。未运行实机 EXE、真实显示器刷新率、硬件/HIL、连接/OTA/debug 后端或签名。

本轮为 Python/PySide6 presentation-only，embedded C/C++ public-source applicability 为 N/A；
架构师 Luna `019ff35f-fbde-7ce2-ac72-f7ef90d5c962`、升级架构师 Terra
`019ff361-cb49-7fa1-81b2-6f20c6b0337b` 与独立 reviewer `019ff366-a4db-7161-a137-04543c61423f`
均超时关闭，未形成外部结论；父代理完成 correctness、architecture、security、performance、
readability 五轴 review 与行为保持简化评估。不作固件标准或认证声明。
