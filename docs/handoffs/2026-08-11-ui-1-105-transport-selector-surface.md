# UI-1.105 传输方式选择器 semantic surface

日期：2026-08-11  
父代理：Codex；父代理是本轮唯一写入者  
共享 checkout：`D:\Workplace\Agent_Workplace\SerialForge`；未检测到 `.git`，未创建/操作 worktree

## 结果与边界

- `QComboBox#transportCombo` 在 base/variant theme owner 中获得 info→input 渐变、强调字重、hover/focus/disabled 状态。
- 六种 `TransportKind`、`itemData`、不可编辑、accessible name、连接 signal、TransportModeSurface glyph 和 runtime 行为均未改变。
- 没有新增状态源、DTO、timer、资源、依赖、传输动作或 OTA/debug 连接。

## 角色与独立复核

```text
产品角色       019fecad-0b92-7df2-bcb7-f6cdd3e2ea1e  called; wait timed out; closed
架构师角色     019fecad-0be4-7443-863f-c53295e59d0e  called; wait timed out; closed
UI 角色        019fecad-0c31-7763-a374-8a8afa6031b3  called; wait timed out; closed
开发角色       019fecad-0c7e-7f93-951f-3595b945deb9  called; wait timed out; closed
验证角色       019fecad-0cca-71e0-a756-c0a730691865  called; wait timed out; closed
打包角色       019fecad-0d19-73b1-a812-d2c62cc83f04  called; wait timed out; closed
独立质量复核   019fecad-b9d4-7fb0-b516-0d494df2641a  called after implementation; wait timed out; closed
```

角色未返回完整报告，超时不视为通过。父代理完成 correctness、readability、architecture、security、performance
五轴审查；简化评估为复用既有通用 combo、ThemeSpec、TransportModeSurface 和连接 builder，不引入第二套 transport state。

## 验证

```text
scripts/check.ps1                                      PASS (154 files <=1000; 3 themes; 22 tokens; 19 selectors; legacy_qss_literals=0)
uv run python -m compileall -q src scripts              PASS
uv run ruff check src scripts                           PASS
UI105_TRANSPORT_SELECTOR_VECTOR_PASS                    themes=3 modes=6 editable=0 item_data=preserved disabled=checked near_white_pixels=0
uv run --locked --extra dev python scripts/provenance.py verify --manifest dist/release/0.1.0/core/onefile/PROVENANCE.json  PASS
```

vector 使用真实 `create_application()` / `create_main_window()` 与 Qt offscreen，未调用 `.show()`；只出现 PySide6 fonts
目录缺失 warning，不影响断言。未运行可见 GUI、EXE startup、读屏/HIDPI/视觉差分、真实 UART/TCP/UDP/BLE/RTT/J-Link、
OTA、签名或硬件验收；未创建、修改或运行 unit test、mock、fixture、harness 或 test-only 资产。

## 打包

```text
artifact: dist/release/0.1.0/core/onefile/app/SerialForge.exe
root: SerialForge.exe (byte-identical; overwritten)
source revision: local-ui-1.105
size: 47,929,551 bytes
SHA-256: D4B68CBE1D4BB00C4C8F35C4BDC8ED9E4B79BE5EC782C865EE39B31D37A1A7B1
archive listing SHA-256: C0E5DAA9B2DA969D1AF5AE10125A207407E19123EC19EAFB3B06C17C82323AC6
python / PyInstaller: 3.12.13 / 6.22.0
signature / release_eligible: NotSigned / false
hardware_acceptance: not_run
```

包是未签名 `engineering_build`，不能等同正式发行或硬件验收通过。

EMBEDDED_WORKFLOW_COMPLETE: review + simplification + verification recorded
