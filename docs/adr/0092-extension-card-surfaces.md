# ADR-0092：扩展工具站能力卡主题表面边界

日期：2026-08-11

状态：已采用

## 背景

嵌入式扩展页已经通过 application DTO 展示 3 个 OTA 传输、2 个 OTA 安全和 2 个 RTT/J-Link 能力，但卡片只设置了 object name/role/state，
缺少显式 `QFrame#extensionCapabilityCard` 与 `QFrame#extensionStationOverview` 主题规则，导致部分环境可能呈现透明或系统原生默认表面。

## 决策

- 在 stable base stylesheet 与 theme variant shell 中同时增加 overview/card selectors，保证三主题 palette 对称。
- 概览使用 info surface + accent 左色带；基础卡片使用 surface/input 渐变；`contract_only` 使用 info surface/blue 左色带，`attach_only` 使用
  history surface/purple 左色带。
- metric value 只使用现有 accent token；所有状态仍来自既有 immutable capability DTO 和 `state` property，presentation 不新增 policy。
- 不引入 hover action、timer、动画、vendor SDK、OTA/debug adapter、图片资源或新 token。

## 结果

扩展站的能力分组更容易扫描，卡片状态与“契约预留/仅附着”语义一致，三主题不再依赖原生 QFrame 默认外观；application/domain/transport
边界和只读/accessibility contract 保持不变。

## 验证

- `scripts/check.ps1`、compileall、Ruff：通过；154 个源文件均不超过 1000 行，3 themes、22 tokens、19 selectors audit 通过。
- Qt offscreen 真实组合根向量：7 cards（5 contract-only、2 attach-only）+ 1 overview，三主题共 24 次内存渲染通过，近白像素为 0。
- 首轮静态门禁发现 `ThemeSpec.border_strong` 不存在；已停止并将通用回退改为已有 `theme.border`，修正后完整门禁与向量通过。
- provenance verifier 通过；onefile canonical artifact 已覆盖项目根目录 `SerialForge.exe`，字节一致。
- 未运行可见 GUI/HIDPI/读屏、真实设备、OTA/debug、硬件、签名或正式发行验收；未创建、修改或运行 unit test、mock、fixture、harness 或 test-only 资产。

嵌入式 C/C++ 适用性：N/A。本轮没有 firmware/MCU/BSP/HAL/RTOS/bootloader/Flash 修改，不声明厂商要求或认证合规。

EMBEDDED_WORKFLOW_COMPLETE: review + simplification + verification recorded
