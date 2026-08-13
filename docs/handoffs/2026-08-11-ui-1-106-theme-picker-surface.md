# UI-1.106 主题选择器 semantic surface

日期：2026-08-11  
父代理：Codex；父代理是本轮唯一写入者  
共享 checkout：`D:\Workplace\Agent_Workplace\SerialForge`；未检测到 `.git`，未创建/操作 worktree

## 结果与边界

- `QComboBox#themePicker` 获得 history→input 渐变、强调字重和 hover/focus/disabled semantic 状态。
- 三个主题 key、三个 ThemeSpec icon、itemData、不可编辑、accessible name、主题切换 signal、一次性 transition 与 palette swatch 均未改变。
- 没有新增状态源、DTO、timer、资源、依赖或设备/OTA/debug 动作。

## 角色与独立复核

```text
产品角色       019fecb1-870e-7f02-adc6-c49894b39078  called; wait timed out; closed
架构师角色     019fecb1-875a-7a71-b9e3-98141376044f  called; wait timed out; closed
UI 角色        019fecb1-87a7-7553-9ab8-6fb17be4012a  called; wait timed out; closed
开发角色       019fecb1-87f4-7153-8db6-afd9f2c05248  called; wait timed out; closed
验证角色       019fecb1-8841-7e00-886b-b366f647b138  called; wait timed out; closed
打包角色       019fecb1-888e-7661-8557-567a6ee57d3b  called; wait timed out; closed
独立质量复核   019fecb2-2741-7c81-9949-34e12155046f  called after implementation; wait timed out; closed
```

角色未返回完整报告，超时不视为通过。父代理完成 correctness、readability、architecture、security、performance
五轴审查；简化评估为复用现有主题 combo、picker icon、palette swatch 和 transition，不新增主题状态模型。

## 验证

```text
scripts/check.ps1                                      PASS (154 files <=1000; 3 themes; 22 tokens; 19 selectors; legacy_qss_literals=0)
uv run python -m compileall -q src scripts              PASS
uv run ruff check src scripts                           PASS
UI106_THEME_PICKER_VECTOR_PASS                          themes=3 keys=3 icons=3 editable=0 theme_signal=preserved disabled=checked near_white_pixels=0
uv run --locked --extra dev python scripts/provenance.py verify --manifest dist/release/0.1.0/core/onefile/PROVENANCE.json  PASS
```

vector 使用真实 `create_application()` / `create_main_window()` 与 Qt offscreen，未调用 `.show()`；只出现 PySide6 fonts
目录缺失 warning，不影响断言。未运行可见 GUI、EXE startup、读屏/HIDPI/视觉差分、真实 UART/TCP/UDP/BLE/RTT/J-Link、
OTA、签名或硬件验收；未创建、修改或运行 unit test、mock、fixture、harness 或 test-only 资产。

## 打包

```text
artifact: dist/release/0.1.0/core/onefile/app/SerialForge.exe
root: SerialForge.exe (byte-identical; overwritten)
source revision: local-ui-1.106
size: 47,929,814 bytes
SHA-256: 82668145CA2F8AA903E3D2BD2A02D8AC4B3B00AAE2DCAD87C9D6F6B42711C2C5
archive listing SHA-256: C0E5DAA9B2DA969D1AF5AE10125A207407E19123EC19EAFB3B06C17C82323AC6
python / PyInstaller: 3.12.13 / 6.22.0
signature / release_eligible: NotSigned / false
hardware_acceptance: not_run
```

包是未签名 `engineering_build`，不能等同正式发行或硬件验收通过。

EMBEDDED_WORKFLOW_COMPLETE: review + simplification + verification recorded
