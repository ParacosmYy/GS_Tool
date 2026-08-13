# ADR 0032：原生 ComboBox popup 顶层 frame 的主题边界

日期：2026-08-10

状态：已接受（UI-1.45）

## 背景

连接快速配置 ComboBox 收起时已经是暗色，但展开 popup 后，Qt 会为 `QComboBox.view().window()` 创建一个独立的顶层
`QFrame`。父窗口 QSS 不会覆盖这个 frame 的默认 palette，导致 popup 上下各出现一条系统白色横带；view/viewport 本身
仍然是主题色，因此普通关闭状态的截图审计无法发现问题。

## 决策

- 新增 `presentation/popup_surface.py`，只查找每个 ComboBox 的原生 `view().window()`，确认它是 `QFrame` 后直接注入
  `render_combo_popup_stylesheet(theme_key)`；刷新通过 `surfaceRole="comboPopup"` 触发既有动态属性 re-polish。
- 新增的 popup stylesheet 只有 frame 的背景、边框和圆角；条目、viewport、scrollbar、键盘导航、焦点和选中行为继续由
  Qt 原生 `QListView` 与既有主题 QSS 负责。
- `theme_stylesheet_controls.py` 提供默认 `QFrame[surfaceRole="comboPopup"]`，`theme_variant_controls.py` 提供
  `ThemeSpec.surface/border` override，`theme_stylesheet_runtime.py` 提供仅 frame 的纯渲染函数。
- `theme.py:apply_theme()` 刷新已存在的 popup；bootstrap 在主窗口组合完成后补一次初始刷新；
  `CommandBatchEditorDialog` 在 combo 构造完成后补一次。主题切换不创建额外动画或 timer。

## 放弃的选项

- 不设置全局 `QApplication` stylesheet，避免改变所有窗口的样式传播、第三方/原生对话框和多窗口生命周期。
- 不关闭原生 popup、不替换 QListView、不创建 QComboBox 子类，也不安装 QApplication 级事件过滤器；这些方案会扩大焦点、键盘和
  Qt style engine 风险。
- 不把完整主窗口 stylesheet复制到每个 popup；popup 只拿 118/208 字节级别的小范围 frame stylesheet，降低重复样式和维护成本。

## 依赖与生命周期

```text
theme.apply_theme / bootstrap / CommandBatchEditor
                    ↓ theme_key
popup_surface.refresh_combo_popup_themes
                    ↓ native view().window() QFrame
theme_stylesheet_runtime.render_combo_popup_stylesheet
                    ↓ ThemeSpec tokens
Qt native QListView popup
```

helper 不保存 popup 引用、不创建 timer、不读取 ViewModel、session、transport 或业务数据。ComboBox 的原生 `view()` 仍是
唯一交互 surface；popup frame 只接收当前主题的 presentation stylesheet。重复刷新幂等，主题切换会替换旧的直接 stylesheet。

## 根因证据

修复前真实 offscreen popup：

```text
popup=465x184; popup palette background=#efefef; near_white rows 1..6 and 177..182;
star_trail/moonlit_ocean/sakura_night near_white=5556
```

修复后：

```text
star_trail     popup=465x186 near_white=0 role=comboPopup
moonlit_ocean  popup=465x186 near_white=0 role=comboPopup
sakura_night   popup=465x186 near_white=0 role=comboPopup
```

## 验证

```text
scripts/check.ps1       PASS 128 files <= 1000; 3 themes; 22 semantic tokens; 14 selectors; Ruff
UI145_IMPORT/COMPILE    PASS 127 modules; compileall
UI145_POPUP_FIXED       PASS three themes; theme switch; popup role; near_white=0
UI145_EDITOR_POPUP      PASS three themes; CommandBatchEditor combo popup; near_white=0
UI145_RESPONSIVE        PASS 980x680 and 1180x780; popup no horizontal/vertical scrollbar; near_white=0
```

已保存复核截图：`build/ui_review_ui145_fixed_star_trail_preset_popup.png`、
`build/ui_review_ui145_fixed_moonlit_ocean_preset_popup.png`、
`build/ui_review_ui145_fixed_sakura_night_preset_popup.png`。

## 复核与限制

- 架构师角色已调用；首轮方案与接入点复核两次等待超时，父级按 popup 根因完成边界审查；import 排序复核返回 GO。
- 独立代码审查覆盖 popup frame / view / runtime / lifecycle / memory；等待结果作为交付前复核证据记录。
- 简化评估：采用小范围 popup frame stylesheet，不复制完整主窗口 stylesheet，不改变原生控件；未为减少行数改变业务行为。
- 离屏环境缺少 PySide6 fonts directory，中文方框不代表 Windows 字体结论；未运行持续 GUI、Windows 原生 style、真实键盘/读屏/HIDPI、
  EXE 启动、硬件、OTA 或 RTT/J-Link 验收。
- 嵌入式 C/C++ 适用性：N/A。本项目是 Python/PySide6 Windows 桌面应用，无 MCU、固件或适用 vendor profile；不声称任何认证合规。
