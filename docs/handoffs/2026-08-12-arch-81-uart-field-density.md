# ARCH-81 / UI-1.154 UART 参数字段密度交接

日期：2026-08-12  
范围：仅 `src/serialforge/presentation/controllers/connection_builder.py` 的 UART presentation
组合。目标是解决 980px 链路页八列网格造成的字段拥挤，同时保持连接业务和用户配置语义不变。

## 实现

- `QGridLayout` 改为三行 `QVBoxLayout`，行内复用 `form_fields.py` 的
  `build_labeled_field()` / `build_field_row()`。
- 第一行：端口 + 波特率；第二行：数据位 + 校验 + 停止位 + 流控；第三行：读超时 + 写超时 +
  字节间超时 + 线路控制。
- 端口 combo 与刷新按钮放进 `port_controls`；独占/DTR/RTS 放进 `line_controls`。这些是局部
  QWidget 组合，不新增业务状态、连接动作或全局 registry。
- 保留 `UartControlBindings`、combo `itemData`、默认值、timeout range、signal callback、连接
  gate、Tab 顺序、accessible name/description、手动输入 COMx 和 `UartTimingSummarySurface`。
- 未修改 `MotionController`、120Hz scheduler、主题 token、domain/application/infrastructure、
  OTA/AES/RTT/J-Link contract-only/attach-only 边界或硬件路径。

## 架构审阅与简化评估

架构师 Luna/max 先行审阅并限定写入面为 `connection_builder.py` 的 UART composition，建议直接
复用现有 `form_fields.py`，不扩展 helper 文件、不触碰调度器或业务层。父代理按此最小边界实现。

独立只读审查线程第一次尝试使用 `git diff`，但 checkout 没有 `.git`，因此无法取得基线差异，首个
结果标记为“证据不足”，不是已确认代码缺陷。父代理随后提供当前源码行段、helper 实现、实际
组合根向量和截图，要求线程直接核对 Qt ownership、绑定不变量与布局风险；补充审阅最终返回
`PASS / Required=0`，Optional 仅保留 `.git` 缺失及未运行单测/硬件的环境限制。父代理完成
presentation 五轴 review：ownership/lifecycle、行为不变量、layout、accessibility、可维护性；
简化结论为保留局部两个容器和既有 helper，不新增抽象层、状态源或 timer。

本轮是 Python/PySide6 presentation-only，embedded C/C++ applicability 为 N/A。没有适用的公开
一手 MCU/SDK/RTOS/driver/OTA 厂商要求；不声称 MISRA、ISO 26262、ASIL、ASPICE 或任何认证合规。
embedded-code-review-simplifier 的固件行为保持/简化检查项为 N/A；UI 代码仍执行独立简化评估。

## 授权的非破坏性验证

```text
uv run ruff check src                         pass
uv run python -m compileall -q src            pass
scripts/check.ps1                             pass
source line limit                             167 files <= 1000
theme token audit                             3 themes / 22 tokens / 0 legacy literals
ARCH81_UART_IMPORT_PASS
ARCH81_DEFAULTS                               baud=115200 data_bits=8 parity=none stop_bits=one flow=none
ARCH81_LABEL_COUNT                            10
ARCH81_FIELD_GEOMETRY_PASS                   10 fields / overlap=0
ARCH81_TAB_ORDER_PASS                        13 target controls
ARCH81_TYPED_VALUE_PASS                      115200/8N1/none -> 230400/7O2/RTS/CTS -> restore
ARCH81_LAYOUT                                 980x720 horizontal_max=0
ARCH81_LAYOUT                                 1240x820 horizontal_max=0
ARCH81_THEME_LAYOUT                           themes=3 horizontal_max=0
ARCH81_RUNTIME_PASS
ARCH81_CLOSE_PASS
```

稳定截图：`C:\Users\Gs\AppData\Local\Temp\serialforge_arch81_uart_980.png`、
`C:\Users\Gs\AppData\Local\Temp\serialforge_arch81_uart_1240.png`。离屏 Qt 仍输出既有
font-directory warning；真实显示器/HIDPI/读屏/硬件连接、传输、刷写、部署与 HIL 未运行，未操作
目标硬件。

## EXE 交付

```text
artifact: dist/release/0.1.0/core/onefile/app/SerialForge.exe
source revision: local-arch-81
size: 47,985,982 bytes
sha256: 81C8993A8D7A8723B074FD0BD37BC7D23822B9BB84070AE12C6E08F26D07B6DA
archive listing sha256: A3E1A7EF48915CD483DC56C5C27D616A894A9F4F45ABDBE82E91382953F24073
canonical/root/root-latest: byte-identical
provenance: pass
signature: NotSigned
release_eligible: false
hardware_acceptance: not_run
root startup/shutdown: pass (PID 45164, exact project-root path)
```

EMBEDDED_WORKFLOW_COMPLETE: review + simplification + verification recorded
