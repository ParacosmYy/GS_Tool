# ADR 0033：文件选择器主题桥与 Qt 文件语义保留

日期：2026-08-10  
状态：已接受（UI-1.46）

## 背景

离屏打开现有 `QFileDialog` 后，主窗口的主题 stylesheet 只能覆盖对话框外壳的一部分；目录树、文件列表和侧栏仍使用
系统 palette。三套主题在 `715×448` 的文件对话框截图中均出现大面积白色区域，最高约 `179,745/320,320` 个像素，
与 ComboBox popup 的独立顶层窗口问题属于同一类“Qt 子表面不继承父窗口 QSS”风险。

## 决策

- 新增 `presentation/file_dialog_surface.py`，提供 `get_open_file_name()` 与 `get_save_file_name()` 两个显式 wrapper，
  让控制器不再直接调用 Qt 的静态 unthemed convenience API。
- wrapper 仍使用 Qt `QFileDialog` 的文件系统模型和标准 `exec()` 生命周期，只映射原有的
  `ExistingFile`/`AnyFile`、`AcceptOpen`/`AcceptSave`、selected path/filter 和取消返回值。
- 为确保 Windows 上 QSS 能作用到 Qt 子控件，桥接显式设置 `DontUseNativeDialog`；这选择 Qt 的可主题化文件对话框外壳，
  不创建自绘文件浏览器，也不改变路径、筛选、键盘、确认和保存覆盖确认语义。
- `theme_stylesheet_controls.py` 提供稳定的 QFileDialog tree/list/header/sidebar/button/scrollbar 规则，
  `theme_variant_controls.py` 提供 `ThemeSpec` override，`theme_stylesheet_runtime.py` 只负责纯渲染。
- helper 在对话框内复用 `refresh_combo_popup_themes()`，因此文件对话框内部的 ComboBox 仍保留原生 popup、键盘、焦点和选中行为。

## 放弃的选项

- 不继续使用 `QFileDialog.getOpenFileName()`/`getSaveFileName()` 静态 API：无法在创建/执行前可靠注入主题 stylesheet。
- 不使用全局 `QApplication` stylesheet 或 palette：会影响第三方/其他顶层窗口并扩大生命周期边界。
- 不实现自绘目录树、文件模型、路径缓存或业务文件服务：这会复制 Qt 文件语义并增加安全/可维护性风险。
- 不把 open/save 状态塞入 ViewModel；wrapper 是无状态 presentation adapter，业务 controller 仍决定文件用途和后续 application 调用。

## 依赖与生命周期

```text
controller (derived_data / replay / terminal_runtime)
                    ↓ open/save intent
presentation.file_dialog_surface
       ├─ theme_key_for_widget(parent)
       ├─ render_file_dialog_stylesheet(theme_key)
       └─ popup_surface.refresh_combo_popup_themes(dialog, theme_key)
                    ↓
             Qt QFileDialog
        (filesystem model + standard dialog semantics)
```

helper 不保存 dialog、path 或 filter，不创建 timer/事件过滤器，不读取 ViewModel/session/transport，也不写文件。

## 根因与修复证据

修复前：

```text
star_trail     QFileDialog 715x448 near_white=179361/320320
moonlit_ocean  QFileDialog 715x448 near_white=172332/320320
sakura_night   QFileDialog 715x448 near_white=179745/320320
```

小范围 QFileDialog QSS 试验后，目录树、列表、侧栏、header、按钮和滚动条均进入主题 surface；固定截图显示 no-white
background fallback。最终三主题桥接截图：

```text
star_trail     QFileDialog 727x436 themed; directory/list no white block
moonlit_ocean  QFileDialog 727x436 themed; directory/list no white block
sakura_night   QFileDialog 727x436 themed; directory/list no white block
```

文件对话框内部 `lookInCombo` 展开仍通过 `comboPopup` frame bridge；open/save wrapper 使用 120ms 取消回调验证均返回
`("", "")`，未创建或写入文件。

## 验证与限制

```text
scripts/check.ps1        PASS 129 files <= 1000; 3 themes; 22 tokens; 19 selectors; Ruff
UI146_IMPORT/COMPILE     PASS 129 modules; compileall; AST pass
UI146_FILE_FIXED         PASS three themes; themed QFileDialog screenshots; internal ComboBox popup themed
UI146_WRAPPER            PASS open/save cancel returns ('', '')
ARCH_REVIEW              architecture role called; worker timed out repeatedly and was closed; parent bounded audit GO
QUALITY_REVIEW           independent read-only reviewer called; timeout/closure recorded; parent five-axis review retained
SIMPLIFICATION           wrapper removes seven duplicated static calls without changing controller/application semantics
```

离屏环境缺少 PySide6 fonts directory，中文显示为方框不代表 Windows 字体结论；Qt offscreen plugin 不支持 raise/keyboard grab，
也不能证明真实 Windows 原生 Shell、HIDPI、读屏或持续 GUI 行为。未运行 unit test、mock、fixture、harness，未操作真实文件、设备或硬件。

嵌入式 C/C++ 适用性：N/A。本项目是 Python/PySide6 Windows 桌面应用，无 MCU、固件或适用的公共 vendor profile；不声称
MISRA、ISO 26262、ASIL、ASPICE 或任何认证合规。
