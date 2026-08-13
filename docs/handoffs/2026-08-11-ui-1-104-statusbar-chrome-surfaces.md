# UI-1.104 QStatusBar 原生 chrome 边界主题化

日期：2026-08-11  
父代理：Codex；父代理是本轮唯一写入者  
共享 checkout：`D:\Workplace\Agent_Workplace\SerialForge`；未检测到 `.git`，未创建/操作 worktree

## 结果与边界

- `theme_stylesheet_controls.py` 增加 `QStatusBar::item` 的透明背景/零边框规则，以及 `QStatusBar QLabel` 的透明背景/零边框/既有 muted 文本规则。
- `theme_variant_controls.py` 对称覆盖相同 selector 的主题颜色；不新增 token、状态源、DTO、timer、资源、依赖或业务动作。
- `StatusFooterSurface` 的 116×18 几何、state/fault/activity/frame/stop API、native status text、accessibility 和 lifecycle owner 均未改变。

## 角色与独立复核

```text
产品角色       019feca8-f7f2-7283-978b-5fa6caf4b70a  called; wait timed out; closed
架构师角色     019feca8-f840-7742-813d-c9341dcb759f  called; wait timed out; closed
UI 角色        019feca8-f88c-7312-8d4c-b76b6f7e0d0b  called; wait timed out; closed
开发角色       019feca8-f8d6-77b1-b520-c3421f4b3afb  called; wait timed out; closed
验证角色       019feca8-f925-7dc3-84a6-bd71e601a5fb  called; wait timed out; closed
打包角色       019feca8-f975-7780-8deb-68fc2aeb4505  called; wait timed out; closed
独立质量复核   019feca9-840d-7ca2-a165-fa869c6ada87  called after implementation; wait timed out; closed
```

角色未返回完整报告，超时不视为通过。父代理完成 correctness、readability、architecture、security、performance
五轴审查；简化评估为复用已有 QStatusBar、ThemeSpec 和 StatusFooterSurface，不建立第二套 footer 状态或动画时钟。

## 验证

```text
scripts/check.ps1                                      PASS (154 files <=1000; 3 themes; 22 tokens; 19 selectors; legacy_qss_literals=0)
uv run python -m compileall -q src scripts              PASS
uv run ruff check src scripts                           PASS
UI104_STATUSBAR_CHROME_VECTOR_PASS                      themes=3 selectors=2 status_footer=116x18 near_white_pixels=0
uv run --locked --extra dev python scripts/provenance.py verify --manifest dist/release/0.1.0/core/onefile/PROVENANCE.json  PASS
```

vector 使用真实 `create_application()` / `create_main_window()` 与 Qt offscreen，未调用 `.show()`；只出现 PySide6 fonts
目录缺失 warning，不影响断言。未运行可见 GUI、EXE startup、读屏/HIDPI/视觉差分、真实 UART/TCP/UDP/BLE/RTT/J-Link、
OTA、签名或硬件验收；未创建、修改或运行 unit test、mock、fixture、harness 或 test-only 资产。

## 打包

```text
artifact: dist/release/0.1.0/core/onefile/app/SerialForge.exe
root: SerialForge.exe (byte-identical; overwritten)
source revision: local-ui-1.104
size: 47,928,792 bytes
SHA-256: 97E43617E1F0B148F713A6049C5338DD3FBF94AE8CE7ACFE9B0F127BCBB8C087
archive listing SHA-256: C0E5DAA9B2DA969D1AF5AE10125A207407E19123EC19EAFB3B06C17C82323AC6
python / PyInstaller: 3.12.13 / 6.22.0
signature / release_eligible: NotSigned / false
hardware_acceptance: not_run
```

包是未签名 `engineering_build`，不能等同正式发行或硬件验收通过。

EMBEDDED_WORKFLOW_COMPLETE: review + simplification + verification recorded
