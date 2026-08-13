# ARCH-6z / UI-1.142 自适应工作区交接

日期：2026-08-11  
父代理：Codex  共享 checkout：`D:\Workplace\Agent_Workplace\SerialForge`

## 本轮目标

修复最小窗口下协议/命令/扩展页首屏被压缩、组件视觉拥挤的问题，同时保持用户已经要求的
120Hz 共享动效、主题切换、低动效和企业级 presentation 边界。

## 实现

`src/serialforge/presentation/controllers/workspace_runtime.py` 的
`on_workspace_tab_changed()` 现在只根据 bounded Tab index 同步既有 focus owner：

- index 0“链路 / 连接”保留默认总览；
- index 1/2/3 自动进入 `set_workspace_focus_mode(window, True)`；
- 回到 index 0 自动恢复 `False`；
- 手动“专注设置 / 返回总览”按钮仍可覆盖当前路由，下一次 Tab 变化允许重新同步。

没有新增 splitter、timer、状态源或业务 callback；下方实时观测、终端和发送区仅做
presentation 隐藏，连接、接收、记录、发送和后台 worker 继续运行。完整决策见
[`ADR 0134`](../adr/0134-adaptive-workspace-focus.md)。

## 真实组合根验证

```text
ADAPTIVE_LAYOUT_PASS 980 720 themes=3 tabs=4
ADAPTIVE_LAYOUT_PASS 1240 820 themes=3 tabs=4
ADAPTIVE_LAYOUT_SUMMARY_PASS 24
REDUCED_MOTION_AUTO_FOCUS_PASS True 542 [False, False, False]
REDUCED_MOTION_AUTO_OVERVIEW_PASS False [True, True, True]
MOTION_TARGET_PASS 120 [8, 9] surfaces=56 motion_children=1
MOTION_CADENCE_MEASURE elapsed_ms=250.0 frames=29 fps=116.0 timer_active=True
scripts/check.ps1: pass
source line limit: pass (165 files <= 1000)
theme token audit: pass (3 themes, 22 semantic tokens, 19 selectors, legacy_qss_literals=0)
```

980×720 总览 workspace 为 150px，配置 focus 为 506/542px，协议/命令/扩展 page viewport
为 435/471px；1240×820 总览为 220px，配置 focus 为 606/642px，协议/命令/扩展 viewport
为 535/571px。四页 horizontal scrollbar maximum 均为 0，root child 无重叠，三主题均通过。
离屏环境仍报告 PySide6 fonts directory warning；这不代表 Windows 系统字体缺失。

## 架构与简化复核

- 架构师：Luna/max/Fast 只读线程已调用，等待后超时并关闭，未返回报告；父代理未把超时当作通过。
- correctness：只在已有 Tab change 边界切换 focus，默认 index 0 与手动按钮行为保持；过渡结束后
  24 组尺寸/主题/Tab 断言通过。
- readability：只增加一个命名常量和一个既有 owner 调用点，没有复制高度动画或可见性策略。
- architecture：workspace runtime 负责路由，focus transition 负责几何，terminal/lifecycle 仍各自拥有
  widget 和状态；无循环依赖、新 facade、splitter 或第二布局策略。
- security：未触及输入边界、密钥、网络、依赖或数据持久化。
- performance：单一 MotionController 仍只有一个 timer child；120Hz target、8/9ms slot 和 250ms
  offscreen 29 帧测量通过；没有新增控件级 timer。

## 包交付

以 `local-arch-6z` 重新生成 onefile，并覆盖根目录 `SerialForge.exe` 与
`SerialForge-latest.exe`。canonical/root/root-latest 均为 `47,976,464` bytes，SHA-256 为
`C7D8D93ED5907648BAF348BD560410BAF0348C5A02705F9880157E68FCD5C9CB`；archive listing SHA-256
为 `FD2322723A63AD7B425D9F9B763D96DB2730E0C558B642548AF0BAEC86323969`。provenance source
revision 为 `local-arch-6z`，签名 `NotSigned`，`release_eligible=false`，硬件验收为 `not_run`。

## Assurance

本轮只改 Python/Qt presentation，public vendor source applicability 为 N/A；不涉及 embedded
C/C++、MCU、vendor SDK、RTOS、ISR/DMA、OTA firmware 或硬件，不宣称 MISRA/ISO 26262/认证合规。
验证为授权的非破坏性 compileall、Ruff、check.ps1、offscreen 组合根与图片观察；真实 Windows GUI
启动、HIDPI、读屏、UART/网络/BLE/RTT/J-Link 和目标板验收未执行。
