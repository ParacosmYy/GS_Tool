# ARCH-92 / UI-1.165 响应式布局与 120Hz 动效交接

日期：2026-08-12  
范围：连接页布局收敛、共享动效 cadence、静态 surface fan-out 优化。  
状态：源码、离屏验证与 onefile 构建完成；根目录旧包已覆盖。

## 已完成

- `scroll_page()` 显式 `AlignTop`，短页面不再把剩余 viewport 高度分配给配置控件。
- `connectionControlBand` 与 UART panel 使用水平 `Expanding`、垂直 `Fixed`，连接带运行时高度
  从此前约 `254px` 收敛到 `138px`；最终 UART panel 样本为 `203px`。
- 保持单一 `MotionController`，`TARGET_HZ=120`、8ms `PreciseTimer`、0.96 nominal frame
  budget；1200ms offscreen sample 为 `145 frames / 120.83Hz`。
- `StatusIndicator`、连接状态 rail、preset context、status footer 增加局部 `motion_active()`；
  lifecycle fan-out 只在 animated frame 且表面明确静态时跳过 repaint，`animated=False` 仍全量
  广播，低动效/暂停/隐藏/关闭/状态切换边界保持。
- 980/1240 连接页可见 scroll 横向范围均为 `0`；四个 workspace 的 visible scroll 逐页检查无
  横向溢出。

## 架构与审查记录

- 布局 owner 仍在 `presentation/controllers/`；surface 只消费投影/帧，不拥有业务状态、timer、
  backend 或线程。没有新增 scroll owner、状态源、循环依赖或 universal abstraction。
- 架构师调用在服务窗口内超时并关闭；独立只读 review 调用同样超时并关闭，未伪造外部 PASS。
  父代理完成 fresh correctness/architecture/performance/security/readability review 和行为保持
  简化评估，未发现 Required finding。
- 本轮为 Python/PySide6 presentation-only；embedded C/C++ public-vendor-source applicability
  为 N/A，不作固件或认证合规声明。

## 验证口径

- pass：`uv run ruff check src scripts`、`uv run python -m compileall -q src`、`scripts/check.ps1`、
  source-limit、theme token audit、真实组合根 offscreen geometry/cadence/lifecycle contract。
- 未运行：EXE 启动、真实显示器/HIDPI/FPS、三主题逐像素回归、硬件/HIL、UART/网络/BLE/RTT 实连、
  OTA/AES 传输和数字签名验收。
- PySide6 offscreen 输出的 `QFontDatabase` font-directory warning 属于当前环境资源提示，未改变
  本轮几何、主题 token 或 cadence 结果。

## 交付物

构建命令：`./scripts/package.ps1 -Mode onefile -SourceRevision local-arch-92`。  
已验证 canonical、根目录 `SerialForge.exe`、根目录 `SerialForge-latest.exe` 三者字节一致，并运行
provenance verify：

- canonical/root/root-latest size: `48,008,038` bytes
- SHA-256: `A2C75788258BB1AE19B23E881049C1AD6F902DD069C78F7E926F15CEF92FD6E8`
- archive listing SHA-256: `032E6E39CDD8EE32A03CC66612F0699685358D5B8C5FB8D6BEAE7BC2C3A8D19E`
- source revision: `local-arch-92`
- signature: `NotSigned` unless an authorized signing step is performed
- `release_eligible=false`
- `hardware_acceptance=not_run`
