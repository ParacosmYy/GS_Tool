# ARCH-76 / UI-1.149 Connection onboarding focus

日期：2026-08-12  
范围：首次未连接启动体验、连接页首屏可见性和 focus/Tab/session 生命周期边界。

## 问题与实现

在 980×720、既有 font runtime 下，workspace 总览只有 `150px`，Tab viewport 约 `79px`，而 connection
control band 约 `149px`。强行提高 shell min-height 会挤压终端并可能越过 root layout；本轮改为复用既有 focus
模式，不创建第二套布局。

- `bootstrap.py` 在 terminal bindings 完成后，将初始 CLOSED 会话交给 `start_connection_onboarding()`；
- `workspace_runtime.py` 只维护 presentation-only `_connection_onboarding`，connection Tab 在 CLOSED 时保持 focus；
- 首次 OPEN 只清除该标志，只有仍在 connection Tab 才回总览；协议/命令/扩展 Tab 保持原有 focus；
- workspace.py 的显式“返回总览”先清除 flag，避免 hide/show 后再次自动 onboarding；
- ERROR/失败不清除 flag，当前连接页可继续修正并重试；历史回放不触发 onboarding；
- 没有新增 timer、MotionController、ViewModel/domain 状态、splitter、事件总线或 OTA/debug coupling。

## 验证证据

```text
ARCH76_ONBOARDING_PASS tabs=507 page=471 connection_band_height=199
ARCH76_MANUAL_OVERVIEW_PASS
ARCH76_OPEN_RETURN_PASS
ARCH76_OPEN_OTHER_TAB_PASS
ARCH76_THEME_ROUTE_PASS size=980x720 themes=3 tabs=4
ARCH76_THEME_ROUTE_PASS size=1240x820 themes=3 tabs=4
ARCH76_THEME_ROUTE_LIFECYCLE_PASS
ARCH76_LIFECYCLE_PASS
ARCH76_SCREENSHOT_PASS onboarding=1 overview=1
scripts/check.ps1: pass
source line limit: pass (166 files <= 1000)
theme token audit: pass (3 themes, 22 semantic tokens, 19 selectors, legacy_qss_literals=0)
uv run ruff check src: pass
uv run python -m compileall -q src: pass
```

稳定截图已检查 onboarding 与回到总览两种状态；onboarding 下 UART 快速连接、UART 参数、波特率预设和
timing summary 均完整可见，没有 viewport 裁切或 root child overlap。offscreen 环境仍会报告 PySide6 缺少
bundled font directory；应用既有 font runtime 后中文显示正常。

## 审查与适用性

架构师只读评审：conditional approve。独立 Luna/max 只读审查：Required=0、Optional=2、FYI=5，未修改文件。
Optional 为重复 start 的幂等 guard，以及 ERROR 状态切 Tab 后的 UX 约定；生产 bootstrap 只有一次 start 调用，
失败重试在当前连接页保留 focus，因此未扩大 diff。

本轮是 Python/PySide6 presentation-only 变更：public MCU vendor source applicability=N/A；无 embedded C/C++、BSP/HAL/RTOS、
硬件写入、刷写、部署或 target 操作。未宣称 MISRA、ISO 26262、ASIL、ASPICE 或任何认证合规。

行为保持型简化评估：复用 `set_workspace_focus_mode()`、已有 focus lifecycle 和 typed workspace bindings；没有
复制 shell 布局、增加状态机或添加专用 timer。onboarding flag 保留在 window presentation 属性中，是跨 bootstrap、
workspace runtime 和 terminal runtime 的最小生命周期契约。

## ARCH-76 包交付

```text
PACKAGE_ARCH76_FINAL                         pass
artifact: dist/release/0.1.0/core/onefile/app/SerialForge.exe
root: SerialForge.exe (byte-identical)
root-latest: SerialForge-latest.exe (byte-identical)
size: 47,982,523 bytes
SHA256: 6607C1BFD68E8B9F5148B19D827D1B000C3C9528DE616D63A7ED5F6E9E3A9AC9
archive listing SHA-256: C9C66CBFDA9102E0A13C6A95DD168ECDF3DC358009DFA125EBCB9D4CF636A732
provenance: pass; source revision local-arch-76
signature: NotSigned; release_eligible=false; hardware_acceptance=not_run
ROOT_EXE_OVERWRITE_PASS: canonical/root/root-latest size and SHA-256 identical
```

根目录交付文件：[`SerialForge.exe`](../../SerialForge.exe)、[`SerialForge-latest.exe`](../../SerialForge-latest.exe)。
