# UI-1.46 交接：QFileDialog 主题桥

日期：2026-08-10  
范围：修复 QFileDialog 目录树/文件列表/侧栏的系统白色 palette 回退，并统一 open/save 调用边界。

## 交付内容

- 新增 [`file_dialog_surface.py`](../../src/serialforge/presentation/file_dialog_surface.py)：
  `configure_file_dialog()`、`get_open_file_name()`、`get_save_file_name()`。
- `derived_data.py`、`replay.py`、`terminal_runtime.py` 不再直接调用 Qt 静态 `QFileDialog` convenience API；wrapper 保持
  `ExistingFile`/`AnyFile`、`AcceptOpen`/`AcceptSave`、selected path/filter 与取消返回语义。
- 新增稳定/主题 override/runtime 三层 QFileDialog QSS，覆盖 tree/list/header/sidebar/button/scrollbar/focus/disabled/selection。
- 文件对话框内部 ComboBox 复用 UI-1.45 的顶层 popup frame bridge；不新增 timer、事件过滤器、业务状态、路径缓存或文件 I/O。
- 明确使用 `DontUseNativeDialog`，以保证 Windows 上 Qt 子控件可主题化；这是外壳选择，不宣称已完成 Windows 原生 Shell/读屏验证。
- 更新 [`ADR 0033`](../adr/0033-themed-file-dialog-surface.md)、[`CONSTRAINTS.md`](../CONSTRAINTS.md)、[`ARCHITECTURE.md`](../ARCHITECTURE.md)、
  `tasks/plan.md`、`tasks/todo.md` 和 `docs/handoffs/current.md`。

## 根因证据

修复前离屏 `715×448` QFileDialog：

```text
star_trail     near_white=179361/320320
moonlit_ocean  near_white=172332/320320
sakura_night   near_white=179745/320320
```

修复后 `727×436` 截图中的目录/列表采样点为主题 surface：

```text
star_trail     sidebar=#17132b; list=#0f0d20; header/border=#7459aa
moonlit_ocean  sidebar=#102638; list=#0a1c2b; header/border=#25495e
sakura_night   sidebar=#29152d; list=#1d0f25; header/border=#5b315e
```

截图：

- `build/ui_review_ui146_fixed_star_trail.png`
- `build/ui_review_ui146_fixed_moonlit_ocean.png`
- `build/ui_review_ui146_fixed_sakura_night.png`
- `build/ui_review_ui146_fixed_star_trail_combo_popup.png`（另两主题同样已生成）

## 验证与审查

```text
scripts/check.ps1        PASS 129 files <= 1000; 3 themes; 22 tokens; 19 selectors; Ruff
UI146_IMPORT/COMPILE     PASS 129 modules; compileall; AST pass
UI146_FILE_FIXED         PASS three themes; themed QFileDialog and internal combo popup
UI146_WRAPPER            PASS open/save cancellation returns ('', '') without file write
provenance verify        PASS local-ui-1.46 artifact manifest
package/root copy        PASS canonical=root; size=47813182; SHA-256=F49C8EEA790D9763C5E9863722A86D8C5AEA16F047FF6B9CECAD9B4831B5B1AF
architecture role        called; worker timed out repeatedly and was closed; parent bounded audit GO
independent review       called; worker timed out and was closed; parent five-axis review recorded
simplification           two explicit stateless wrappers replace seven duplicated static calls
embedded applicability    N/A; Python/PySide6 desktop, no MCU/C/C++/firmware/vendor profile
```

限制：离屏环境缺少 PySide6 fonts directory，中文方框不代表 Windows 字体结论；offscreen plugin 不支持 raise/keyboard grab；未运行持续
GUI、EXE 启动、Windows 原生 Shell、HIDPI/读屏、真实设备/硬件、OTA/RTT/J-Link 或正式签名发布验收。未创建、修改或运行 unit test、
mock、fixture、harness 或 test-only 资产；`signature=NotSigned`、`release_eligible=false`、`hardware_acceptance=not_run`。

EMBEDDED_WORKFLOW_COMPLETE: review + simplification + verification recorded
