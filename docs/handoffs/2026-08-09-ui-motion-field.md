# SerialForge UI-1.1 方向无关信号场交接

日期：2026-08-09  
范围：presentation-only 共享动效生命周期、顶部几何信号场、组件微交互和曲线绘制修复  
状态：in progress（本轮静态门通过；最终二次元视觉方向、GUI/EXE、重新打包和硬件仍未验收）  
父代理：Codex  
共享 checkout：`D:\Workplace\Agent_Workplace\SerialForge`；未检测到 `.git`，未创建/操作 Git/Codex worktree

## 用户目标与范围

用户要求持续美化 SerialForge UI 组件和动态效果，所有验收要点交付成功后才停止。本轮遵守“不持续启动软件”、只改 presentation、动效可暂停/reduced-motion、未选定二次元视觉方向不得固化美术风格的边界。

## 本轮用户可见结果

- 顶部壳层新增无资源、固定尺寸、鼠标穿透的几何信号场；它只是氛围装饰，不表达连接、吞吐、RX 或错误真值。
- 状态指示器和信号场消费同一 `MotionController` 帧；RX 活动只合并一个短活动窗口，连接 opening/closing 可持续驱动。
- “低动效”、`SERIALFORGE_REDUCED_MOTION=1/true/yes/on` 和“暂停动效”同时覆盖信号场与状态脉冲；静态点/静态场保留，业务接收、记录、回放和终端预览不受影响。
- 窗口隐藏、最小化和关闭会停止共享 timer；恢复时重新同步当前连接状态。
- 补充按钮、工具按钮、Tab、复选框的焦点可见性，补充 `QDialog` 根背景并提高 subtle 文本 token；修复 Dataset 曲线发光线后主线未重置起点的潜在闭合线。

## 实际改动文件

- `src/serialforge/presentation/widgets.py`：新增 `MotionController`、`SignalFieldWidget`；`StatusIndicator` 移除自持 timer，改为共享帧消费者。
- `src/serialforge/presentation/main_window.py`：接入信号场、统一动效策略、RX activity 请求和 hide/minimize/close 生命周期。
- `src/serialforge/presentation/theme.py`：更新 `TEXT_SUBTLE`、`QDialog` 背景和焦点规则。
- `src/serialforge/presentation/dataset_curve.py`：重置主线循环的 `previous`。
- `README.md`、`docs/ROADMAP.md`：说明共享动效控制器和顶部信号场仍属于方向无关基础层。
- `handoff.md`：更新最新交接入口。

未修改 domain/application/infrastructure、ViewModel 信号语义、transport、protocol、recorder、replay、queue 或硬件行为；未新增运行时依赖、图片/GIF/字体/QRC 或 PyInstaller 数据路径。

## 六角色只读评审

| 角色 | agent id | 结果 | 父代理处理 |
|---|---|---|---|
| 产品 | `019fe692-2bbb-7e62-99b4-c74c4e6f6115` | revise | 允许方向无关信号场先行；限定在 header、复用两个现有开关、保留静态回退且不承载业务含义。 |
| 架构 | `019fe692-2bfb-7c62-aff6-41117e305d3d` | revise | 指出隐藏/最小化后排队信号可能重启 timer；采用单一 `MotionController` 和 suspended/closed/activity 闸门。 |
| UI 设计 | `019fe692-2c3b-7272-bc3f-70b3b73d8e7a` | revise | 要求信号场不覆盖内容、持续 RX 聚合、补齐焦点态；均已纳入。 |
| 开发 | `019fe692-2c79-7762-b8b9-78e5c70e0964` | revise | 发现曲线 glow/main line 起点未重置和 dialog 背景缺失；已修复。 |
| 验证 | `019fe692-2cb5-7013-9548-23aea49d0cce` | revise | 建议静态、导入、边界和共享 timer 审计；本轮已完成静态证据，GUI/包门仍未授权。 |
| 打包/流程 | `019fe692-2cf8-72e0-9463-f03216fb2b6f` | revise | 确认无新增资源打包路径；旧四包只能标记 `observed-historical`，本轮未重包。 |

六角色均为 `luna_max / max / Fast`、只读；父代理为唯一写入者。所有角色未启动 GUI/EXE/服务、未连接硬件、未创建或运行测试专用资产。

## 架构、简化与行为保持

- `MotionController(QObject)` 是唯一 presentation 动效时钟；timer 间隔 96 ms，仅在过渡态或 activity deadline 内运行。
- `SignalFieldWidget` 和 `StatusIndicator` 不读取 ViewModel、不保存业务状态、不创建自己的 timer；信号场设置 `WA_TransparentForMouseEvents` 和 `NoFocus`。
- 低动效/暂停/隐藏/最小化/关闭均为控制器闸门；暂停冻结静态帧，reduced-motion 停止连续刷新但保留可读静态界面。
- `request_activity()` 不接收或解析 payload，只延长截止时间；不把动效状态写入 domain/application。
- 本轮是 Python desktop presentation 变更，嵌入式 C/C++ assurance applicability=N/A；不据此声称 MISRA、ISO、认证或硬件合规。

## 验证证据

| 命令/检查 | 实际输出 | 状态 |
|---|---|---|
| `.venv\Scripts\ruff.exe check src scripts` | `All checks passed!` | verified |
| `.venv\Scripts\ruff.exe format --check src scripts` | `55 files already formatted` | verified |
| Python `-B` 内存 `compile()` | `compiled 55 files` | verified |
| Python `-B` 全包导入 | `imported 53 serialforge modules` | verified |
| AST presentation/lifecycle 审计 | `presentation boundary and shared-motion static audit: passed` | verified |

AST 审计确认 presentation 没有直接导入 `serial`、`bleak`、`socket`，非 presentation 没有 Qt 越层，`MotionController` 只创建一个 `QTimer`，MainWindow 不再调用旧 `pulse()`。

## 未运行项目、授权和风险

- GUI/offscreen、HIDPI、焦点顺序、读屏、实际 QSS 状态、最小窗口、帧率/CPU/内存和隐藏/最小化运行行为：`not-run`，因仓库约束和用户“不持续启动软件”要求未授权。
- EXE 启动、当前源码重新打包、四矩阵 provenance/archive/签名：`not-run`；既有 `dist/release/0.1.0` 只能作为 `observed-historical`，不代表当前源码。
- UART/BLE/Wi-Fi/TCP/UDP/J-Link、真实设备和网络：`not-run`，未请求也未操作硬件。
- unit test、mock、fixture、harness 和测试专用资产：未创建、未修改、未运行。
- 未决视觉风险：用户尚未选择二次元候选，因此没有角色、插画、GIF、粒子或具体资源方向。顶部信号场是可替换的方向无关基础层，不应被称为最终美术背景。

## 下一步与用户选择

1. 用户选择一个视觉候选，并确定色板、密度、动效强度、资源来源/许可证、静态回退和是否允许 GUI/offscreen 验证。
2. 下一轮重新执行六角色，只在 presentation/assets 接入选定 backdrop；若加入外置资源，再按授权重跑 core/BLE × onedir/onefile 打包矩阵。
3. 取得 GUI/offscreen 授权后验证 `980×680`、HIDPI、键盘/读屏、禁用/错误/空态、信号场层级和 timer 生命周期。

当前不能宣称完整 UI、最终二次元背景、GUI/EXE、真实硬件或正式发行通过。
