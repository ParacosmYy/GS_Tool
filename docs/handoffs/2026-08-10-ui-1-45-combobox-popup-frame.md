# UI-1.45 交接：ComboBox popup 顶层 frame 白带修复

日期：2026-08-10  
父代理：Codex；共享 checkout 唯一写入者  
范围：presentation theme/popup；未接触 domain/application/infrastructure/transport/hardware

## 用户可见结果

展开连接快速配置时，顶部/底部的系统白色横带已消失；三套主题的 popup frame 都使用当前主题的暗色表面和边框。
原生 ComboBox popup、QListView 条目、键盘导航、焦点和选中行为保留。

## 根因与变更

Qt 的 `QComboBox.view().window()` 是独立顶层 `QFrame`，不继承主窗口的 widget QSS；它的默认 palette 是 `#efefef`，
即使 `QComboBox QAbstractItemView` 和 viewport 已经暗色，popup 外壳仍在展开时显示上下白带。

- `src/serialforge/presentation/popup_surface.py`
  - 新增 `refresh_combo_popup_theme()` 和 `refresh_combo_popup_themes()`。
  - 只对 `view().window()` 的 `QFrame` 设置小范围 popup stylesheet，并复用 `refresh_dynamic_property()` 设置
    `surfaceRole=comboPopup`。
  - 不保存 popup 引用、不创建 timer、不接管原生交互。
- `src/serialforge/presentation/theme_stylesheet_controls.py`
  - 新增默认 `COMBO_POPUP_STYLESHEET`，只定义 frame 背景/边框/圆角。
- `src/serialforge/presentation/theme_variant_controls.py`
  - 新增 `build_combo_popup_theme_override(theme)`，使用 `ThemeSpec.surface/border`。
- `src/serialforge/presentation/theme_stylesheet_runtime.py`
  - 新增纯函数 `render_combo_popup_stylesheet(theme_key)`，不复制完整主窗口 stylesheet。
- `src/serialforge/presentation/theme.py`
  - 每次 `apply_theme()` 刷新已有 ComboBox popup，覆盖主题切换。
- `src/serialforge/presentation/controllers/bootstrap.py`
  - 主窗口 central widget 完成后补一次 popup 初始化刷新。
- `src/serialforge/presentation/command_batch_editor.py`
  - 编辑器内部 quick combo 构造后补一次 popup 初始化刷新。
- `scripts/check_theme_tokens.py`
  - popup frame selector 加入 14 项主题覆盖门禁。

## 架构审查

- 首轮 UI-1.45 架构师：`019fea89-ed9c-7af2-8912-5912d4f3c094`，两次等待超时关闭；父级依据复现证据确认方案。
- runtime 依赖整理架构师：`019fea8d-3c60-7111-abf3-e311052f8593`，超时后关闭；父级确认 `theme_spec` 明确导入、无循环依赖。
- 接入点/导入顺序架构师：`019fea8d-99ff-7c22-a16b-1a11875fd21a`，确认 import 调整为 GO，并确认三个接入点仍在。

父级审查结论：不采用 QApplication 全局 stylesheet、QComboBox 子类或事件过滤器；popup frame 是独立 presentation 变化边界，
主题 runtime 提供小范围 QSS，业务层无任何改动。

## 验证证据

```text
旧 popup（基线）       465x184；上下近白 rows=1..6/177..182；三主题 near_white=5556
UI145_POPUP_FIXED      PASS 三主题；theme switch；popup=465x186；role=comboPopup；near_white=0
UI145_EDITOR_POPUP     PASS 三主题；编辑器 popup=263x39；near_white=0
UI145_RESPONSIVE       PASS 980x680、1180x780；hscroll=False；vscroll=False；near_white=0
UI145_IMPORT           PASS 127 modules；无 QApplication instance
UI145_COMPILE          PASS python -m compileall -q src
scripts/check.ps1      PASS 128 files <= 1000；3 themes；22 tokens；14 selectors；Ruff
```

截图：

- 基线：`build/ui_review_ui145_star_trail_preset_popup.png`
- 修复：`build/ui_review_ui145_fixed_star_trail_preset_popup.png`
- 修复：`build/ui_review_ui145_fixed_moonlit_ocean_preset_popup.png`
- 修复：`build/ui_review_ui145_fixed_sakura_night_preset_popup.png`

离屏环境警告：缺少 `PySide6/lib/fonts`，中文方框不代表 Windows 字体缺失。未运行持续 GUI、EXE、真实 Windows Qt style、
读屏、HIDPI、硬件、OTA、RTT/J-Link 或正式签名验收；未创建、修改或运行测试专用资产。

## 嵌入式 assurance gate

适用性 N/A：SerialForge 是 Python/PySide6 Windows 桌面应用，无 MCU、C/C++、固件、BSP/HAL/CMSIS、RTOS 或硬件目标；
无适用厂商要求，不声称 MISRA、ISO 26262、ASIL、ASPICE 或认证合规。独立复核、简化评估和非破坏性验证已记录。

## 最终打包

- canonical：[dist/release/0.1.0/core/onefile/app/SerialForge.exe](../../dist/release/0.1.0/core/onefile/app/SerialForge.exe)
- 根目录覆盖：[SerialForge.exe](../../SerialForge.exe)，路径已校验在当前 workspace 内
- source revision：`local-ui-1.45`
- size：`47,809,326` bytes
- canonical/root SHA-256：`56B2B61A79A1CB997990D1AC96FACCEEFAEB035FF9D6F88F73E21ACF29473D28`
- archive listing SHA-256：`91A2F43B3917E69DE58A7545F5DB682447B4594F50B82E1A9D0A8B36343E7BAB`
- provenance：`PROVENANCE.json` verify pass；`signature=NotSigned`；`release_eligible=false`；`hardware_acceptance=not_run`
- 未启动 EXE；只验证 canonical/root size 与 SHA-256 一致。

EMBEDDED_WORKFLOW_COMPLETE: review + simplification + verification recorded
