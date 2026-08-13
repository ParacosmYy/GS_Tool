# ADR 0052：工作区原生 Tab 主题化语义 glyph

## 状态

已接受（UI-1.65，2026-08-10）。

## 背景

工作区已经有原生 `QTabWidget` 和独立的 `WorkspaceRouteSurface` 路线 beacon，但 Tab 只有文字标签，链路、协议和命令三个
工作域在紧凑窗口中缺少快速的视觉语义。新增视觉提示不能再引入一套自绘导航控件，也不能破坏用户依赖的原生 Tab 键盘、焦点
和无障碍行为。

## 决策

新增 `presentation/workspace_tab_icons.py` 作为 presentation-only glyph owner。它使用 `QPainter` 绘制三个固定、有界的矢量
图标，并按照当前 `ThemeSpec` 为每个图标配置 `QIcon.Mode.Normal`、`Selected` 和 `Disabled` pixmap。`workspace.py` 在三个
Tab 组装完成后调用一次，`lifecycle.py` 在主题应用完成后刷新一次；少于三个 Tab 或尚未组装时安全返回。

图标只增强视觉识别，不替换原生标签文字，不写入 ViewModel，不保存导航/业务状态，不创建 timer，不使用外部图片或第三方
依赖。原生 `QTabWidget` 继续拥有 Tab index、`currentChanged`、焦点、键盘导航和 accessibility 语义；路线 beacon 仍是独立
的装饰 surface。

## 未采用的方案

- emoji 或文字图标：字体、平台和主题渲染不稳定，且会与可读 Tab 文案/无障碍重复。
- 完全自绘 TabBar：可获得更多视觉控制，但会重新承担原生焦点、键盘、可访问性和选中态契约，边界明显扩大。
- 外部 SVG/PNG 资源：需要资源打包与高 DPI 变体管理，当前三个固定 glyph 的矢量绘制不需要额外依赖。

## 影响与验证

正向影响是三个工作域获得轻量、主题一致的语义入口，同时保持原生导航契约和文件职责边界。图标在主题刷新时会重新生成
三态 pixmap；这是低频、三个 Tab 的有界操作，不进入 MotionController，也不增加常驻动画。

已完成 `UI165_TAB_ICON_VECTOR_PASS`：三套 `ThemeSpec` 下三个 Tab 图标非空且可取得 18×18 实际尺寸，原始中文标签逐项保持。
已完成 Ruff、compileall、项目静态检查与模块导入验证；GUI/offscreen/HIDPI/读屏/EXE 启动和真实硬件验收未在本轮授权范围内运行。
