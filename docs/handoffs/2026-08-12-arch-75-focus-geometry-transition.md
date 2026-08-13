# ARCH-75 / UI-1.148 Focus geometry transition

日期：2026-08-12  
范围：工作区专注模式的高度过渡、窗口 resize 生命周期与 120Hz 动效不变量。

## 问题与实现

动态采样发现，旧实现只把 `tabs.maximumHeight` 从 `350` 动画到 `_MAX_HEIGHT`。当最大值越过
布局可用高度时，Qt 会把页面从约 `192px` 直接撑到约 `614px`，中间帧表现为表单挤压/突跳。

`presentation/workspace_focus_transition.py` 现在：

- 优先读取已布局 widget 的真实 `height()`，只有构造早期高度为零时才回退到 size hint；
- 进入 focus 时隐藏 live/terminal/send，重新激活 root layout，测量 focus 目标，再对当前真实高度到目标高度做 `maximumHeight` 动画；
- 退出总览时先测量 tabs 与下方 surface 的总览目标，再从当前真实高度做 reveal；
- 静态回退统一清理 `minimumHeight`、`maximumHeight` 和 `visible` 约束，不改变 session、接收、记录、发送或 Tab 语义。

`presentation/controllers/lifecycle.py` 的 `resizeEvent()` 在调用 Qt 基类重排前停止 focus geometry transition，
避免窗口尺寸改变后动画继续写入旧目标；既有主题过渡清理仍保持在同一生命周期边界内。

## 验证证据

```text
ARCH75_GEOMETRY_PASS
  before=192px; focus 实测连续扩展至 614px；不再出现 350 -> 614px 哨兵突跳
ARCH75_THEME_SIZE_PASS size=980x720 themes=3 tabs=4
ARCH75_THEME_SIZE_PASS size=1240x820 themes=3 tabs=4
ARCH75_RESIZE_ABORT_PASS
ARCH75_LOW_MOTION_PASS
ARCH75_LIFECYCLE_PASS hidden=True closed=True
ARCH75_MOTION_SCHEDULE_PASS hz=120 timer=PreciseTimer intervals=[8,8,9,8,8,9,8,8,9,8,8,9]
ARCH75_SCREENSHOT_FONT_PASS overview=1 protocol=1 extension=1
scripts/check.ps1: pass
source line limit: pass (166 files <= 1000)
theme token audit: pass (3 themes, 22 semantic tokens, 19 selectors, legacy_qss_literals=0)
uv run ruff check src: pass
uv run python -m compileall -q src: pass
```

稳定截图已检查总览、协议/遥测和扩展/工具站：980×720 下无页面/route 重叠、无横向滚动，中文字体
通过应用既有 font runtime 正常显示。offscreen 环境仍会报告 PySide6 缺少 bundled font directory；这是既有
环境提示，不是本轮主题或布局失败。

## 审查与适用性

独立 Luna/max 代码审查最终返回 Required=0、Optional=0、FYI=1；确认 resize stop 顺序、静态约束清理、
动画 identity guard、快速反转、hide/minimize/close 和低动效路径。Optional 的静态 `layout.activate()` 未采用，
因为现有静态约束在验证中已收敛。

本轮是 Python/PySide6 presentation-only 变更：public MCU vendor source applicability=N/A；没有 embedded C/C++、
BSP/HAL/RTOS、硬件写入、刷写、部署或 target 操作。未宣称 MISRA、ISO 26262、ASIL、ASPICE 或任何认证合规。
架构师调用按用户约束执行但未在等待窗口内返回独立结论，未将超时作为 review evidence；独立审查与父代理验证证据
分别记录，避免混淆角色。

## ARCH-75 包交付

```text
PACKAGE_ARCH75_FINAL                         pass
artifact: dist/release/0.1.0/core/onefile/app/SerialForge.exe
root: SerialForge.exe (byte-identical)
root-latest: SerialForge-latest.exe (byte-identical)
size: 47,981,610 bytes
SHA256: 6AA8BFE552E357D9FA88B05B8070E42A17CB9B535FD85B93866CEE3C383A1C7E
archive listing SHA-256: C9C66CBFDA9102E0A13C6A95DD168ECDF3DC358009DFA125EBCB9D4CF636A732
provenance: pass; source revision local-arch-75
signature: NotSigned; release_eligible=false; hardware_acceptance=not_run
ROOT_EXE_OVERWRITE_PASS: canonical/root/root-latest size and SHA-256 identical
```

根目录交付文件：[`SerialForge.exe`](../../SerialForge.exe)、[`SerialForge-latest.exe`](../../SerialForge-latest.exe)。
