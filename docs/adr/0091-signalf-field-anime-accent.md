# ADR-0091：SignalField 二次元星芒与共享动效边界

日期：2026-08-11

状态：已采用

## 背景

SerialForge 已有资源无关的 `SignalFieldWidget`，用于 Header 中的串口信号视觉提示。原始画面只有网格、轨道椭圆、扫描线和光点，主题色已经
具备二次元方向，但角色感和动效层次仍偏弱。直接引入图片、GIF 或独立动画 timer 会增加资源发布、主题回退和生命周期风险。

## 决策

- 保持 `SignalFieldWidget` 的尺寸、无障碍和 `set_frame()/stop()` API 不变，在现有 QPainter 内增加三枚四点星芒、移动光点彗尾和轨道光点层。
- 星芒位置和动态漂移只使用共享 `phase/animated`；静态帧使用固定位置、低透明度，保证 reduced-motion/隐藏/关闭后的视觉可解释性。
- 所有颜色必须从 `ThemeSpec` 的 accent/pink/blue/purple token 取得；不写白色 literal、不加载图像/字体、不创建新的 timer、线程或跨层状态。
- `workspace.py` 继续负责组装，`lifecycle.py` 继续通过 `_motion_surfaces()` 统一分发帧与 stop；组件不读取 view model、domain、transport、OTA 或 debug。

## 结果

新增视觉层次而不改变任何业务信息、连接状态、可访问文本、主题选择、窗口生命周期或扩展站契约。后续更换视觉方向仍只需调整
presentation painter 和 ThemeSpec，不会污染 application/domain/adapter 层。

## 验证

- `scripts/check.ps1`、compileall、Ruff：通过；154 个源文件均不超过 1000 行，三主题和 semantic token audit 通过。
- Qt offscreen 真实组合根向量：三主题 × 静态/动态帧共 6 次内存渲染通过，`near_white_pixels=0`，共享 motion surface 与 stop 通过。
- 首次内联向量仅因验证脚本访问 Qt 枚举的方式错误而失败；修正脚本后复跑通过，未修改测试资产或产品代码。
- provenance verifier 通过；onefile canonical artifact 已覆盖项目根目录 `SerialForge.exe`，字节一致。
- 未运行可见 GUI/HIDPI/读屏、真实设备、OTA/debug、硬件、签名或正式发行验收；未创建、修改或运行 unit test、mock、fixture、harness 或 test-only 资产。

嵌入式 C/C++ 适用性：N/A。本轮没有 firmware/MCU/BSP/HAL/RTOS/bootloader/Flash 修改，不声明厂商要求或认证合规。

EMBEDDED_WORKFLOW_COMPLETE: review + simplification + verification recorded
