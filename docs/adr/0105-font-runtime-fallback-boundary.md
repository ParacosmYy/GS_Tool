# ADR-0105：字体运行时 fallback 边界

日期：2026-08-11  
状态：accepted（UI-1.117）

## 背景

SerialForge 的 QSS 已声明 Windows 优先字体族，但某些 Qt offscreen/plugin 环境的字体数据库为空，
中文会退化为方框，削弱视觉审计和受限运行环境的可读性。项目是 Windows-first，不应为了审计把字体
文件复制进仓库或发行物，也不应让字体问题进入业务层。

## 决策

- 新增 `presentation/font_runtime.py`，在 `QApplication` 创建后由 `presentation/main.py` 调用。
- 先选择已存在的 `Microsoft YaHei UI`、`Microsoft YaHei`、`Noto Sans SC`、`Noto Sans CJK SC`
  或 `Segoe UI`；只有在没有 CJK family 时，才检查标准本机字体路径并调用
  `QFontDatabase.addApplicationFont()`。
- 不复制、下载、写入用户字体目录或打包字体资产；注册只存在于当前 Qt 进程。
- 注册失败或没有 family 时无条件保留 Qt 默认并继续启动；stylesheet 用有序 fallback 提供第二层保障。

## 备选方案

### 将字体文件加入 PyInstaller

拒绝：增加发行物体积和字体授权/许可证责任，也让 core 包绑定单一字体资产。

### 在每个控件中设置 QFont

拒绝：分散字体生命周期，容易与 QSS、主题切换和对话框继承产生分裂，并扩大 controller 责任。

### 忽略受限环境的方框

拒绝：真实 Windows 可能已有字体，但离屏审计无法观察中文布局和裁切；bounded 本机 fallback 能在不
改变业务边界的前提下提高证据质量。

## 后果与风险

正常 Windows 运行优先复用系统字体，字体文件不会进入发行物。受限环境可能读取标准本机字体文件，
这属于本地只读/进程内注册；路径缺失或注册失败不会阻断启动。不同 Windows 安装的字体家族名称、
字体授权和 Qt platform plugin 仍需在授权的干净 Windows/HIDPI/读屏环境验收，本 ADR 不声明字体
覆盖或发行资格。

## 验证

`UI117_FONT_RUNTIME_PASS family=Microsoft YaHei UI families=2`；
`UI117_AUDIT_RENDER_PASS`（3 themes × 4 tabs，1180×780）；
`UI117_RESPONSIVE_VECTOR_PASS`（3 themes × 4 tabs，980×680，无横向溢出）；
source limit、compileall、Ruff、theme token audit 和 provenance verify 均通过。架构师与独立质量
审查线程在限定窗口内超时，未计为通过；父代理完成正确性、边界、失败回退、复用/简化和行数审查。
本轮不包含嵌入式 C/C++，embedded applicability=N/A。
