# ADR-0090：Pipeline 状态语义与共享动效边界

日期：2026-08-11

状态：已采用

## 背景

`PipelineSurfaceLabel` 已经通过生命周期接收 `state/source` 动态属性，也接收共享 `MotionController` 的 `phase/animated`，但此前
所有状态都绘制相同的四节点全量彩色轨道，并在收到 animated frame 时统一 pulse。这样 UI 看不出实时/历史、草稿/已应用、连接流程或阻塞
状态的差异，也容易让装饰性动画被误认为业务进度。

## 决策

- 保留 `lifecycle.py` 作为 `state/source` 的写入 owner；`pipeline_surface.py` 不读取 view model、domain、transport 或事件总线。
- 在 presentation surface 内维护一个有界、纯函数式的状态映射：
  - `active=4`、`history=4`、`transition=3`、`draft=2`、`blocked=1`、`idle=1` 个激活节点；未激活节点降低透明度。
  - history 使用历史紫色语义；active/transition/draft 使用既有 accent token；blocked 使用 subdued token 和静态叉标。
  - 仅 active/transition/draft 消费共享 phase 绘制 pulse；history/blocked/idle 始终静态。
- 非法 `state/source` 只允许安全回退到 `idle/live`，不抛异常、不修改业务状态。
- 继续使用 `ThemeSpec` 和既有 `_motion_surfaces()` fan-out；不新增 QTimer、线程、业务 DTO、设备能力、依赖或资源。

## 结果

该 surface 与现有 lifecycle 状态事实保持一致，主题切换和 reduced-motion/隐藏/最小化/关闭 fence 继续由既有 owner 管理。
Pipeline 文本、accessible description、QSS properties、application/domain contract、OTA/debug contract 均不变。状态映射局部化后，后续
新增视觉状态只需在这个 bounded mapping 和 lifecycle contract 中明确评审，避免把业务进度塞进装饰层。

## 验证

- `scripts/check.ps1`、compileall、Ruff：通过；154 个源文件均不超过 1000 行，三主题 semantic token audit 通过。
- Qt offscreen 真实组合根向量：三主题 × 六状态 × 两类来源，36 次内存渲染通过；非法 property fail-open、moving gate 和 stop freeze 通过。
- `scripts/provenance.py verify`：通过；onefile canonical artifact 已覆盖项目根目录 `SerialForge.exe`，字节一致。
- 未运行可见 GUI/HIDPI/读屏、真实 UART/TCP/UDP/BLE/RTT/J-Link、OTA、硬件、签名或正式发行验收；未创建、修改或运行 unit test、mock、fixture、harness 或 test-only 资产。

嵌入式 C/C++ 适用性：N/A。本轮没有 firmware/MCU/BSP/HAL/RTOS/bootloader/Flash 修改，不声明厂商要求或认证合规。

EMBEDDED_WORKFLOW_COMPLETE: review + simplification + verification recorded
