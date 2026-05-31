# PRD-040: NavIndicatorWidget + ToastWidget 应用集成

## 背景

NavIndicatorWidget (src/core/NavIndicatorWidget.h, 195行) 和 ToastWidget (src/core/ToastWidget.h, 186行) 已作为独立 header-only 组件实现完成。NavIndicatorWidget 提供导航树选中项的 accent 色竖线滑动动画（250ms OutCubic），ToastWidget 提供右下角弹出通知（300ms OutBack 弹入，250ms InCubic 消失，支持 Success/Error/Info 三种类型）。

当前状态:
- NavIndicatorWidget 已在 MainWindow::setupUI() 中创建并作为 navTree 的子控件覆盖（第174行），导航树点击时已调用 moveToIndex()（MainWindowSignalConnect.cpp 第192行）。
- ToastWidget 的静态 show() 方法已在 MainWindowSignalConnect.cpp 中用于连接失败（第88行）、发送状态消息（第105行）、录制/回放消息（第142行）。
- 但仍有以下集成缺口未覆盖。

缺口清单:
1. NavIndicatorWidget 初始化时未调用 jumpToIndex() 设置初始指示线位置，启动时指示线不可见。
2. 会话恢复（restorePanelByIndex）后未同步 NavIndicatorWidget 位置。
3. 连接状态变化（Connected/Disconnected/Error）未显示 ToastWidget 通知，仅有连接失败的 toast。
4. OTA 传输完成/失败未显示 ToastWidget 通知。
5. 窗口 resize 时 NavIndicatorWidget 通过 eventFilter 已自动跟随 navTree 尺寸，但 themeChanged 后指示线颜色通过 ThemeManager::themeChanged 信号已自动刷新，无需额外处理。
6. 串口热插拔（portAdded/portRemoved）仅更新状态栏文字，缺少 toast 视觉反馈。

本 PRD 覆盖上述全部集成缺口，将两个组件完整接入应用信号链。

## 需求列表

| ID | 需求描述 | 优先级 | 涉及模块 |
|----|---------|--------|---------|
| R1 | NavIndicatorWidget 初始化: 在 buildNavTree + setCurrentPanel 之后，获取终端面板对应的 QModelIndex 并调用 jumpToIndex() 设置初始指示线位置 | P0 | MainWindow.cpp |
| R2 | 会话恢复同步: restorePanelByIndex() 执行后，根据恢复的面板索引计算对应 navTree 的 QModelIndex，调用 jumpToIndex() 同步指示线位置 | P0 | MainWindow.cpp |
| R3 | 连接成功 Toast: ConnectionState::Connected 时显示 ToastWidget::show(parent, tr("已连接: %1").arg(connName), ToastType::Success) | P0 | MainWindow.cpp handleConnectionState() |
| R4 | 连接断开 Toast: ConnectionState::Disconnected 时显示 ToastWidget::show(parent, tr("连接已断开"), ToastType::Info) | P1 | MainWindow.cpp handleConnectionState() |
| R5 | 连接错误 Toast: ConnectionState::Error 时显示 ToastWidget::show(parent, tr("连接错误"), ToastType::Error) | P0 | MainWindow.cpp handleConnectionState() |
| R6 | OTA 传输完成 Toast: OtaManager::transferComplete 信号触发时显示 ToastWidget::show(parent, tr("OTA传输完成"), ToastType::Success) | P0 | MainWindowSignalConnect.cpp |
| R7 | OTA 传输失败 Toast: OtaManager::transferError 信号触发时显示 ToastWidget::show(parent, tr("OTA传输失败: %1").arg(reason), ToastType::Error) | P0 | MainWindowSignalConnect.cpp |
| R8 | 串口热插拔 Toast: portAdded 时显示 Info toast，portRemoved 时显示 Info toast | P2 | MainWindowSignalConnect.cpp |

## 接口设计

### 无新增类或接口

本次 PRD 纯属集成工作，不新增任何类、方法或信号。所有改动均在现有方法内部添加调用。

### 涉及的现有接口

**NavIndicatorWidget (已存在，无需修改)**:
- `jumpToIndex(const QModelIndex&)` -- 无动画跳转，用于初始化和会话恢复
- `moveToIndex(const QModelIndex&)` -- 带动画滑动，导航树点击时已连接

**ToastWidget (已存在，无需修改)**:
- `static show(QWidget* parent, const QString& message, ToastType type, int durationMs)` -- 唯一公开接口

**MainWindow (修改现有方法)**:
- `handleConnectionState()` -- 增加 toast 调用（R3/R4/R5）
- `MainWindow()` 构造函数 -- 增加 jumpToIndex() 初始位置设置（R1）
- 构造函数中 restorePanelByIndex 分支 -- 增加指示器同步（R2）

**MainWindowSignalConnect.cpp connectSignals()** -- 增加信号连接（R6/R7/R8）

### NavIndicatorWidget 初始位置计算方式

由于导航树模型在 buildNavTree() 中创建（QStandardItemModel），终端面板是串口分组的第二个子项（索引 0=配置, 1=终端）。获取初始 QModelIndex 的方法:

```cpp
// 在 buildNavTree + setCurrentPanel 之后执行
QAbstractItemModel* model = m_navTree->model();
if (model && model->rowCount() > 0) {
    QModelIndex serialRoot = model->index(0, 0);           // "串口" 分组
    QModelIndex terminalIdx = model->index(1, 0, serialRoot); // "终端" 子项
    m_navIndicator->jumpToIndex(terminalIdx);
}
```

会话恢复时同理，根据 lastPanel 索引遍历 navTree 模型查找对应叶子节点的 QModelIndex。

## 依赖的公共组件

| 组件 | 文件 | 用途 |
|------|------|------|
| NavIndicatorWidget | core/NavIndicatorWidget.h | 导航指示器，header-only，已存在 |
| ToastWidget | core/ToastWidget.h | 通知吐司，header-only，已存在 |
| ThemeManager | core/ThemeManager.h/cpp | 语义颜色（两个 widget 内部已引用） |
| NavigationController | core/NavigationController.h/cpp | 面板映射表、索引查找 |
| OtaManager | ota/OtaManager.h | OTA 状态信号 |
| ConnectionController | core/ConnectionController.h | 连接状态信号、热插拔信号 |

## 设计模式

无新增设计模式。本次集成沿用现有模式:

- **观察者模式 (Observer)**: 通过 Qt 信号/槽连接，将 OtaManager::transferComplete/transferError、ConnectionController::connectionStateChanged、PortWatcher::portAdded/portRemoved 信号路由到 ToastWidget::show() 调用。
- **中介者模式 (Mediator)**: MainWindow 作为中介者协调信号路由，不包含业务逻辑。

## 影响范围

### 修改文件

| 文件 | 改动内容 | 预估行数 |
|------|---------|---------|
| MainWindow.cpp | handleConnectionState() 增加 toast 调用（R3/R4/R5），构造函数末尾增加 NavIndicatorWidget 初始化（R1/R2） | +15行 |
| MainWindowSignalConnect.cpp | connectSignals() 增加 OTA 信号连接（R6/R7）和热插拔 toast（R8） | +15行 |

### 对现有功能的影响

- **handleConnectionState()**: 仅增加 toast 调用，不修改现有逻辑（状态栏更新、呼吸动画、面板切换均保持不变）。新增 toast 不会与现有 connectionFailed toast 冲突，因为 connectionFailed 是独立信号，对应 QMessageBox 弹窗场景。
- **connectSignals()**: 仅新增 connect() 调用，不修改现有连接。
- **构造函数**: 仅在现有初始化流程末尾追加 jumpToIndex() 调用，不影响已有初始化顺序。
- **NavIndicatorWidget/ToastWidget**: 两个组件代码不做任何修改。

### 不影响的功能

- 面板切换动画（NavigationController::switchToPanel 保持不变）
- 主题切换过渡（ThemeManager::themeChanged 信号已连接）
- 窗口 resize（NavIndicatorWidget 的 eventFilter 已处理）
- Toast 垂直堆叠（ToastWidget 内部 activeToasts + repositionToasts 已处理）

## 验收标准

### 功能验收

- [ ] AC1: 应用启动后，导航树"终端"项左侧可见 accent 色竖线指示器（无动画，直接显示）
- [ ] AC2: 点击导航树其他项时，指示器平滑滑动到新位置（250ms OutCubic 动画流畅）
- [ ] AC3: 关闭应用并重新打开后，指示器恢复到上次活跃面板对应的位置
- [ ] AC4: 串口连接成功后，右下角弹出绿色 Success toast 显示"已连接: COMx"，3秒后自动消失
- [ ] AC5: 串口断开连接后，右下角弹出 Info toast 显示"连接已断开"，3秒后自动消失
- [ ] AC6: 连接错误时，右下角弹出红色 Error toast 显示"连接错误"，3秒后自动消失
- [ ] AC7: OTA 传输完成后，右下角弹出绿色 Success toast 显示"OTA传输完成"
- [ ] AC8: OTA 传输失败时，右下角弹出红色 Error toast 显示"OTA传输失败: {原因}"
- [ ] AC9: 多条 toast 同时显示时垂直堆叠，间距 8px，消失后自动重排位置
- [ ] AC10: 切换暗色/亮色主题后，指示线颜色立即更新为新主题的 accent 色
- [ ] AC11: 窗口 resize 时指示器跟随 navTree 尺寸变化，不出现错位或裁剪

### 非功能验收

- [ ] AC12: 所有动画帧率 >= 30fps，无卡顿掉帧
- [ ] AC13: 编译零错误零警告
- [ ] AC14: MainWindow.cpp 行数不因此次改动突破 500 行上限
- [ ] AC15: 无新增类、无新增文件，仅修改 MainWindow.cpp 和 MainWindowSignalConnect.cpp

### 排除项（不在本 PRD 范围内）

- ToastWidget 关闭按钮交互（当前设计为自动消失，无手动关闭）
- NavIndicatorWidget 悬浮预览效果（点击后才滑动，悬浮不触发）
- Toast 持久化或历史记录
