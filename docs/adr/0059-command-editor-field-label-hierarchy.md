# ADR 0059：批量命令编辑器字段标签使用主题次级层级

日期：2026-08-10

状态：accepted

## 背景

批量命令编辑对话框的普通字段标签和固定顺序说明仍是没有语义属性的原生 `QLabel`。这让对话框在主题切换后依赖系统
palette 的默认文字层级，字段、编辑控件、空态和错误状态的视觉关系不稳定，并可能出现白色回退。

## 决策

在 `presentation/command_batch_editor.py` 内增加局部 `_field_label(text)` helper，只创建 `QLabel` 并设置
`role="muted"`。以下 5 个静态标签通过该 helper 组装：

- 名称
- 快捷命令
- 固定顺序 · 无循环/脚本/广播；宏只在当前运行中保存
- 当前步骤
- 延时

`_table_empty` 继续使用 `role="subtle"`，错误 label 继续使用 `role="error"`。helper 不拥有 draft、bounded text、校验、
popup、按钮 action、焦点或 Tab 顺序，不新增依赖、状态、timer 或跨页面 label 服务。

## 未采用方案

- 不修改全局 `QLabel` 默认样式：会误伤空态、错误和动态状态，且无法表达对话框字段边界。
- 不引入共享表单构建器：当前只需在一个局部 dialog owner 中复用 5 次，通用配置会增加耦合。
- 不增加装饰动画或固定颜色：字段层级是静态主题语义，动效应继续由既有生命周期 owner 管理。

## 验证

- 三套 ThemeSpec 的短时 Qt offscreen dialog vector：5 个目标标签均为 `role="muted"`；`subtle`/`error` 角色保持。
- `.venv\Scripts\python.exe -m compileall -q src`：pass。
- `scripts/check.ps1` 与 onefile/provenance 会在本轮最终交付门禁中执行。
- 独立质量复核代理 `019feb93-267a-7f63-8d16-d9eff1471964` 在窗口内超时并关闭；未将超时写成通过，父代理完成五轴审查。
- 未启动完整 GUI/EXE、未运行 unit tests、未创建测试/夹具/模拟器、未接入真实设备；嵌入式 C/C++ 适用性：N/A。
