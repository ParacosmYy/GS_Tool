# UI-1.103 工作区外壳与路线装饰条主题表面

日期：2026-08-11  
父代理：Codex；父代理是本轮唯一写入者  
共享 checkout：`D:\Workplace\Agent_Workplace\SerialForge`；未检测到 `.git`，未创建/操作 worktree

## 结果与边界

- 在 `theme_stylesheet_controls.py` 为 `QFrame#workspaceShell` 增加明确的 surface、border 与圆角。
- 为 `QFrame#workspaceRouteStrip` 增加 history→info 语义渐变、上下边界和底部圆角，消除透明/原生默认回退。
- `theme_variant_controls.py` 以同名 selector 只覆盖 `ThemeSpec` 语义 token；没有新增 palette、DTO、状态源、timer、依赖或动作。
- `workspace.py`、`WorkspaceRouteSurface`、Tab index、导航文案、焦点/无障碍和共享 MotionController 生命周期均未改变。

## 角色与独立复核

```text
产品角色       019feca0-29a7-7152-bf48-500e54ebe6eb  called; wait timed out; closed
架构师角色     019feca0-29ea-7691-9da2-77d60fccbdc3  called; wait timed out; closed
UI 角色        019feca0-2a36-7f21-8fda-baae92eeecdb  called; wait timed out; closed
开发角色       019feca0-2a8a-7333-be06-58325f697d7b  called; wait timed out; closed
验证角色       019feca0-2ad8-7381-8cef-c62d01fef58d  called; wait timed out; closed
打包角色       019feca0-2b20-7f31-8ec4-ca6422626144  called; wait timed out; closed
独立质量复核   019feca0-e3cf-7902-a2a3-579cb6f3a4af  called after implementation; wait timed out; closed
```

角色未返回完整报告，超时不视为通过。父代理完成 correctness、readability、architecture、security、performance
五轴审查：样式集中在 presentation theme owner，复用既有 token，未把视觉策略带回 controller；简化评估为没有
新增通用主题层或第二套导航/动效机制。

## 验证

```text
scripts/check.ps1                                      PASS (154 files <=1000; 3 themes; 22 tokens; 19 selectors; legacy_qss_literals=0)
uv run python -m compileall -q src scripts              PASS
uv run ruff check src scripts                           PASS
UI103_WORKSPACE_SURFACE_VECTOR_PASS                    themes=3 selectors=2 tabs=4 route=148x28 near_white_pixels=0
uv run --locked --extra dev python scripts/provenance.py verify --manifest dist/release/0.1.0/core/onefile/PROVENANCE.json  PASS
```

vector 使用真实 `create_application()` / `create_main_window()` 与 Qt offscreen，未调用 `.show()`。第一次裸 `python`
因环境未导入 PySide6，第二次验证调用了错误的 `QWidget.render` 签名，第三次只在清理阶段错误调用无参 session close；
这些都是验证入口问题，修正后最终向量通过，产品代码未因它们增加补丁。离屏环境仍提示 PySide6 fonts 目录缺失，
不影响本轮颜色/结构断言。未运行可见 GUI、EXE startup、读屏/HIDPI/视觉差分、真实 UART/TCP/UDP/BLE/RTT/J-Link、
OTA、签名或硬件验收；未创建、修改或运行 unit test、mock、fixture、harness 或 test-only 资产。

## 打包

```text
artifact: dist/release/0.1.0/core/onefile/app/SerialForge.exe
root: SerialForge.exe (byte-identical; overwritten)
source revision: local-ui-1.103
size: 47,928,961 bytes
SHA-256: 4F0300C4764F94F25825C8B5795D348BD04D1AFF8A43E6078B7676FA909B70D1
archive listing SHA-256: C0E5DAA9B2DA969D1AF5AE10125A207407E19123EC19EAFB3B06C17C82323AC6
python / PyInstaller: 3.12.13 / 6.22.0
signature / release_eligible: NotSigned / false
hardware_acceptance: not_run
```

包是未签名 `engineering_build`，不能等同正式发行或硬件验收通过。

EMBEDDED_WORKFLOW_COMPLETE: review + simplification + verification recorded
