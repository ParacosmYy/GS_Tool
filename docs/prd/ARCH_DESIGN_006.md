# 架构设计文档 ARCH_DESIGN_006

> 设计人: 系统架构师 (system-architect)
> 设计日期: 2026-05-31
> 关联约束: CLAUDE.md 4.6 (MainWindow.cpp 必须 < 500 行)
> 涉及模块: core/MainWindow, core/NavigationController, core/RecordingController
> 预计变更: ~380 行新增 (两个新类), ~420 行从 MainWindow.cpp 迁出
> 目标: MainWindow.cpp 从 1092 行降至 ~400 行

---

## 一、设计背景

CLAUDE.md 4.6 节规定 `.cpp` 文件不得超过 500 行, `MainWindow.cpp` 当前 1092 行, 违反铁律。
MainWindow 作为"上帝对象"承担了过多职责:

| 职责域 | 涉及成员 | 估计行数 |
|-------|---------|---------|
| 导航树构建 + 面板映射 + 面板切换动画 | m_navTree, m_navPanelMappings, m_currentPanel, m_panelSwitching, switchToPanel, allSwitchablePanels, buildNavPanelMappings, setupUI 中导航树部分 | ~170 |
| 录制/回放控制 | m_recordAction, m_stopRecordAction, m_playbackAction, m_stopPlaybackAction, onToggleRecording, onStopRecording, onOpenPlayback, onStopPlayback, onPlaybackData, onPlaybackProgress, onRecordingStopped | ~130 |
| 连接管理 | m_connManager, m_currentConn, onConnectSerial, onDisconnectSerial, onConnectNetwork | ~120 |
| 终端/数据交互 | sendAndRecord, onSendData, onDataReceived, onQuickCommand | ~100 |
| UI 布局 (setupUI 剩余部分) | m_mainSplitter, m_rightPanel, 各面板 new, 发送区域, 快捷指令栏 | ~200 |
| 主题/语言/状态栏/设置 | onThemeChanged, onLanguageChanged, setupStatusBar, updateStatusBar, loadSettings, saveSettings | ~170 |
| 呼吸动画 | startBreathingAnimation, stopBreathingAnimation, m_breathingAnim, m_connStatusEffect | ~60 |
| 信号连接 | connectSignals | ~120 |

本次设计拆出两个最大且内聚性最高的职责域:

1. **NavigationController** -- 导航树 + 面板切换 + 淡入淡出动画
2. **RecordingController** -- 日志录制/回放的完整生命周期

拆分后 MainWindow.cpp 预计 ~400 行, 满足 < 500 行的约束。

---

## 二、NavigationController 设计

### 2.1 类图 (ASCII)

```
                    QObject
                      |
           NavigationController
                      |
    +-----------------+------------------+
    |                 |                  |
 NavPanelMapping   QTreeView*       QWidget*
 (数据结构)        (外部创建,        (当前面板追踪)
                   引用不拥有)

signals:
  panelSwitchRequested(QWidget*)
```

### 2.2 头文件设计

```cpp
// src/core/NavigationController.h
#ifndef NAVIGATION_CONTROLLER_H
#define NAVIGATION_CONTROLLER_H

#include <QObject>
#include <QVector>
#include <QPixmap>

class QTreeView;
class QWidget;

// 导航树 -> 面板映射条目
struct NavPanelMapping {
    const char* name;       // 翻译键, 传给 tr() 进行运行时翻译匹配
    QWidget* widget;        // 目标面板指针
};

class NavigationController : public QObject {
    Q_OBJECT

public:
    explicit NavigationController(QObject* parent = nullptr);
    ~NavigationController() override;

    // 构建导航树模型 (在 MainWindow::setupUI 中调用, 传入已创建的 QTreeView)
    void buildNavTree(QTreeView* navTree);

    // 注册面板映射表 (在所有面板创建完成后调用)
    void buildNavPanelMappings(
        QWidget* serialConfig,
        QWidget* terminal,
        QWidget* dataStats,
        QWidget* protocolView,
        QWidget* frameEditor,
        QWidget* chartWidget,
        QWidget* otaWidget
    );

    // 切换到指定面板 (带淡出/淡入动画)
    void switchToPanel(QWidget* newPanel);

    // 获取当前显示的面板
    QWidget* currentPanel() const;

    // 收集所有可切换面板 (用于全部隐藏)
    QVector<QWidget*> allSwitchablePanels() const;

    // 设置初始面板 (用于构造后指定默认面板)
    void setCurrentPanel(QWidget* panel);

signals:
    // 面板切换完成后发出, MainWindow 可用于同步 UI 状态
    void panelSwitched(QWidget* newPanel);

    // 功能性节点点击信号 (不走面板切换, 直接触发动作)
    void exportRequested();
    void networkConnectRequested(int connectionType);  // ConnectionType 枚举值

private slots:
    void onNavTreeClicked(const QModelIndex& index);

private:
    // 创建连接类型指示圆点图标
    static QIcon createDotIcon(const QColor& color);

    QTreeView* m_navTree = nullptr;
    QVector<NavPanelMapping> m_navPanelMappings;
    QWidget* m_currentPanel = nullptr;
    bool m_panelSwitching = false;

    // 缓存面板指针 (用于 allSwitchablePanels)
    QWidget* m_serialConfig = nullptr;
    QWidget* m_terminal = nullptr;
    QWidget* m_dataStats = nullptr;
    QWidget* m_protocolView = nullptr;
    QWidget* m_frameEditor = nullptr;
    QWidget* m_chartWidget = nullptr;
    QWidget* m_otaWidget = nullptr;
};

#endif // NAVIGATION_CONTROLLER_H
```

### 2.3 方法签名与职责

| 方法 | 来源 (MainWindow.cpp 行号) | 职责 |
|------|---------------------------|------|
| `buildNavTree(QTreeView*)` | 第 81-145 行 (导航树模型构建) | 创建 QStandardItemModel, 添加串口/网络/工具分组, 展开全部 |
| `buildNavPanelMappings(...)` | 第 347-359 行 | 构建 name->widget 映射表 |
| `switchToPanel(QWidget*)` | 第 368-466 行 | 淡出旧面板 + 淡入新面板动画, 防重入 |
| `currentPanel()` | 第 43 行 (m_currentPanel 读取) | 返回当前面板指针 |
| `allSwitchablePanels()` | 第 362-366 行 | 返回所有可切换面板列表 |
| `setCurrentPanel(QWidget*)` | 第 43 行 (m_currentPanel 赋值) | 设置初始面板 (不触发动画) |
| `onNavTreeClicked(QModelIndex)` | 第 559-582 行 (lambda) | 处理导航树点击: 功能节点发信号, 面板节点调 switchToPanel |
| `createDotIcon(QColor)` | 第 63-73 行 (static 函数) | 创建 8x8 彩色圆点图标 |

### 2.4 信号/槽连接

```
NavigationController 内部连接:
  m_navTree->clicked  -->  onNavTreeClicked(QModelIndex)

NavigationController 发出的信号 --> MainWindow 接收:
  panelSwitched(QWidget*)            -->  (预留, 可用于同步 UI 状态)
  exportRequested()                  -->  MainWindow::onExportData()
  networkConnectRequested(int)       -->  MainWindow::onConnectNetwork(ConnectionType)
```

NavigationController 不持有 DataLogger, ConnectionManager 等业务对象。
它只负责"用户在导航树上点了什么 -> 应该显示哪个面板"。

---

## 三、RecordingController 设计

### 3.1 类图 (ASCII)

```
                         QObject
                            |
                 RecordingController
                            |
          +-----------------+------------------+
          |                 |                  |
     DataLogger*       QAction* x4        (无子对象)
     (外部创建,        (工具栏按钮,
      引用不拥有)      外部创建)

signals:
  recordingToggled(bool)
  playbackProgress(qreal)
  statusMessage(QString)
  playbackData(QByteArray, qint64)
  recordingStopped(QString, int, qint64)
```

### 3.2 头文件设计

```cpp
// src/core/RecordingController.h
#ifndef RECORDING_CONTROLLER_H
#define RECORDING_CONTROLLER_H

#include <QObject>
#include <QByteArray>

class QAction;
class QToolBar;
class DataLogger;
class QLabel;

class RecordingController : public QObject {
    Q_OBJECT

public:
    explicit RecordingController(DataLogger* dataLogger,
                                  QObject* parent = nullptr);
    ~RecordingController() override;

    // 创建并添加录制/回放按钮到工具栏 (在 MainWindow::setupToolbar 中调用)
    void setupActions(QToolBar* toolbar);

    // 状态查询
    bool isRecording() const;
    bool isPlaying() const;

    // 停止所有活动 (用于 closeEvent)
    void stopAll();

signals:
    // 录制状态变化
    void recordingToggled(bool active);

    // 回放进度
    void playbackProgress(qreal percent);

    // 状态栏消息
    void statusMessage(const QString& msg, int timeoutMs = 0);

    // 回放数据 (需要注入到终端)
    void playbackData(const QByteArray& data, qint64 direction);

    // 录制完成
    void recordingStopped(const QString& filePath, int count, qint64 durationMs);

private slots:
    void onToggleRecording(bool checked);
    void onStopRecording();
    void onOpenPlayback();
    void onStopPlayback();
    void onPlaybackDataReceived(const QByteArray& data, qint64 direction);
    void onPlaybackProgressReceived(qreal percent);
    void onRecordingStoppedReceived(const QString& filePath, int count, qint64 durationMs);
    void onPlaybackFinished();
    void onLoggerError(const QString& msg);

private:
    DataLogger* m_dataLogger;  // 外部拥有, 不负责销毁

    QAction* m_recordAction = nullptr;
    QAction* m_stopRecordAction = nullptr;
    QAction* m_playbackAction = nullptr;
    QAction* m_stopPlaybackAction = nullptr;

    // 用于 QFileDialog 的父窗口
    QWidget* m_parentWidget() const;
};

#endif // RECORDING_CONTROLLER_H
```

### 3.3 方法签名与职责

| 方法 | 来源 (MainWindow.cpp 行号) | 职责 |
|------|---------------------------|------|
| `RecordingController(DataLogger*, QObject*)` | -- | 构造, 保存 DataLogger 引用 |
| `setupActions(QToolBar*)` | 第 289-299 行 | 创建录制/停止录制/回放/停止回放按钮, 添加到工具栏 |
| `isRecording()` | -- | 代理 m_dataLogger->isRecording() |
| `isPlaying()` | -- | 代理 m_dataLogger->isPlaying() |
| `stopAll()` | closeEvent 第 998-999 行 | 停止录制 + 停止回放 (供 closeEvent 调用) |
| `onToggleRecording(bool)` | 第 1012-1038 行 | 录制/暂停/恢复 切换逻辑 |
| `onStopRecording()` | 第 1041-1047 行 | 停止录制, 恢复按钮状态 |
| `onOpenPlayback()` | 第 1049-1060 行 | 打开日志文件, 开始回放 |
| `onStopPlayback()` | 第 1062-1067 行 | 停止回放, 恢复按钮状态 |
| `onPlaybackDataReceived(...)` | 第 1069-1077 行 | 转发回放数据到终端 |
| `onPlaybackProgressReceived(qreal)` | 第 1079-1082 行 | 转发回放进度到状态栏 |
| `onRecordingStoppedReceived(...)` | 第 1084-1092 行 | 转发录制完成信息到状态栏 |
| `onPlaybackFinished()` | 第 502-506 行 | 恢复按钮状态 |
| `onLoggerError(QString)` | 第 507-509 行 | 显示错误消息 |

### 3.4 信号/槽连接

```
RecordingController 内部连接:
  m_recordAction->toggled(bool)       -->  onToggleRecording(bool)
  m_stopRecordAction->triggered()     -->  onStopRecording()
  m_playbackAction->triggered()       -->  onOpenPlayback()
  m_stopPlaybackAction->triggered()   -->  onStopPlayback()
  m_dataLogger->playbackData          -->  onPlaybackDataReceived
  m_dataLogger->playbackProgress      -->  onPlaybackProgressReceived
  m_dataLogger->recordingStopped      -->  onRecordingStoppedReceived
  m_dataLogger->playbackFinished      -->  onPlaybackFinished()
  m_dataLogger->error                 -->  onLoggerError(QString)

RecordingController 发出的信号 --> MainWindow 接收:
  playbackData(QByteArray, qint64)    -->  MainWindow::onPlaybackData
                                               (注入 TerminalModel)
  statusMessage(QString, int)         -->  statusBar()->showMessage(...)
  recordingStopped(QString,int,qint64)-->  statusBar()->showMessage(...)
```

关键设计决策: RecordingController 不直接操作 TerminalModel 和 statusBar。
它通过信号将回放数据和状态消息发出, 由 MainWindow 决定如何展示。
这保持了 RecordingController 在业务层的定位, 不跨层依赖表现层。

---

## 四、MainWindow 重构后的结构

### 4.1 移除的成员 (MainWindow.h)

```cpp
// 以下成员从 MainWindow.h 中删除:

// 导航相关
QVector<NavPanelMapping> m_navPanelMappings;
QWidget* m_currentPanel = nullptr;
bool m_panelSwitching = false;
// (QTreeView* m_navTree 保留, 因为 setupUI 中仍需创建它)

// 录制/回放按钮
QAction* m_recordAction;
QAction* m_stopRecordAction;
QAction* m_playbackAction;
QAction* m_stopPlaybackAction;
```

### 4.2 新增的成员 (MainWindow.h)

```cpp
// 新增成员:
NavigationController* m_navController;      // 导航控制
RecordingController* m_recordingController; // 录制/回放控制
```

### 4.3 移除的方法 (MainWindow.h)

```cpp
// 以下方法从 MainWindow.h 中删除:
void buildNavPanelMappings();
void switchToPanel(QWidget* newPanel);
QVector<QWidget*> allSwitchablePanels() const;

// 以下 private slots 从 MainWindow.h 中删除:
void onToggleRecording();
void onStopRecording();
void onOpenPlayback();
void onStopPlayback();
void onPlaybackData(const QByteArray& data, qint64 direction);
void onPlaybackProgress(qreal percent);
void onRecordingStopped(const QString& filePath, int count, qint64 duration);
```

### 4.4 保留的方法 (MainWindow.h)

```cpp
// MainWindow 保留的职责:
void setupUI();                    // 创建所有 QWidget, 布局
void setupToolbar();               // 创建工具栏 (调用 m_recordingController->setupActions)
void setupStatusBar();             // 创建状态栏
void connectSignals();             // 信号连接编排
void updateStatusBar();            // RX/TX 字节刷新
void updateDataStatistics();       // 数据统计刷新
void loadSettings();               // 加载设置
void saveSettings();               // 保存设置
bool sendAndRecord(...);           // 统一发送方法
void startBreathingAnimation();    // 呼吸动画
void stopBreathingAnimation();

// 保留的 slots:
void onConnectSerial();
void onDisconnectSerial();
void onSendData();
void onQuickCommand(const QByteArray& data);
void onDisplayModeChanged(int index);
void onTimestampToggled(bool checked);
void onClearTerminal();
void onExportData();
void onSearchRequested(...);
void onSearchCleared();
void onConnectNetwork(ConnectionType type);
void onThemeChanged(int index);
void onLanguageChanged(int index);
void onConnectionStateChanged(ConnectionState state);
void onDataReceived(const QByteArray& data);
```

---

## 五、逐行迁移计划

### 5.1 NavigationController 迁移

| MainWindow.cpp 行号 | 内容 | 迁移目标 | 动作 |
|---------------------|------|---------|------|
| 63-73 | `createDotIcon()` 静态函数 | NavigationController.cpp | 移动 (改为 private static 方法) |
| 81-145 | 导航树模型构建 (setupUI 内) | NavigationController::buildNavTree() | 提取为新方法 |
| 347-359 | `buildNavPanelMappings()` | NavigationController::buildNavPanelMappings() | 移动并修改签名 (接收面板指针) |
| 362-366 | `allSwitchablePanels()` | NavigationController::allSwitchablePanels() | 移动 |
| 368-466 | `switchToPanel()` | NavigationController::switchToPanel() | 移动 |
| 559-582 | 导航树 clicked lambda | NavigationController::onNavTreeClicked() | 提取为 slot 方法 |

MainWindow.cpp 变更:
- 第 81-145 行: 替换为 `m_navController->buildNavTree(m_navTree);`
- 第 347-359 行: 替换为 `m_navController->buildNavPanelMappings(...);`
- 第 362-466 行: 整段删除 (方法移到 NavigationController)
- 第 559-582 行: 替换为 `connect(m_navTree, &QTreeView::clicked, m_navController, ...);`  -- 实际上由 NavigationController 内部连接

### 5.2 RecordingController 迁移

| MainWindow.cpp 行号 | 内容 | 迁移目标 | 动作 |
|---------------------|------|---------|------|
| 289-299 | 录制/回放按钮创建 (setupToolbar 内) | RecordingController::setupActions() | 提取为新方法 |
| 492-509 | 录制信号连接 (connectSignals 内) | RecordingController 构造函数 | 移动到 RecordingController 内部 |
| 890 | `switchToPanel(m_terminal)` 在连接状态中 | 改为 `m_navController->switchToPanel(m_terminal)` | 调用目标变更 |
| 998-999 | closeEvent 中停止录制/回放 | `m_recordingController->stopAll()` | 替换 |
| 1012-1038 | `onToggleRecording()` | RecordingController::onToggleRecording() | 移动 |
| 1041-1047 | `onStopRecording()` | RecordingController::onStopRecording() | 移动 |
| 1049-1060 | `onOpenPlayback()` | RecordingController::onOpenPlayback() | 移动 |
| 1062-1067 | `onStopPlayback()` | RecordingController::onStopPlayback() | 移动 |
| 1069-1077 | `onPlaybackData()` | RecordingController 内部 + 信号转发 | 移动 |
| 1079-1082 | `onPlaybackProgress()` | RecordingController 内部 + 信号转发 | 移动 |
| 1084-1092 | `onRecordingStopped()` | RecordingController 内部 + 信号转发 | 移动 |

MainWindow.cpp 变更:
- 第 289-299 行: 替换为 `m_recordingController->setupActions(m_toolbar);`
- 第 492-509 行: 替换为 RecordingController 的信号连接
- 第 890 行: `switchToPanel(m_terminal)` 改为 `m_navController->switchToPanel(m_terminal)`
- 第 998-999 行: 替换为 `m_recordingController->stopAll();`
- 第 1012-1092 行: 全部删除 (方法移到 RecordingController)

### 5.3 connectSignals() 重写

原有的 connectSignals() 中的录制信号连接 (第 492-509 行) 全部移入 RecordingController 内部。
MainWindow 的 connectSignals() 新增以下连接:

```cpp
// NavigationController -> MainWindow
connect(m_navController, &NavigationController::exportRequested,
        this, &MainWindow::onExportData);
connect(m_navController, &NavigationController::networkConnectRequested,
        this, [this](int type) { onConnectNetwork(static_cast<ConnectionType>(type)); });

// RecordingController -> MainWindow
connect(m_recordingController, &RecordingController::playbackData,
        this, [this](const QByteArray& data, qint64 dir) {
    if (dir == 0) m_terminalModel->appendReceived(data);
    else m_terminalModel->appendSent(data);
    updateStatusBar();
});
connect(m_recordingController, &RecordingController::statusMessage,
        this, [this](const QString& msg, int timeout) {
    statusBar()->showMessage(msg, timeout);
});
connect(m_recordingController, &RecordingController::recordingStopped,
        this, [this](const QString& info) {
    statusBar()->showMessage(info, 5000);
});
```

---

## 六、预期 MainWindow.cpp 行数分析

### 6.1 迁出行数统计

| 迁出内容 | 行数 |
|---------|------|
| createDotIcon() | 11 |
| 导航树模型构建 (setupUI 内) | 65 |
| buildNavPanelMappings() | 13 |
| allSwitchablePanels() | 5 |
| switchToPanel() 含动画 | 99 |
| 导航树 clicked lambda | 24 |
| 录制/回放按钮创建 (setupToolbar 内) | 11 |
| 录制信号连接 (connectSignals 内) | 18 |
| onToggleRecording() | 27 |
| onStopRecording() | 7 |
| onOpenPlayback() | 12 |
| onStopPlayback() | 6 |
| onPlaybackData() | 9 |
| onPlaybackProgress() | 4 |
| onRecordingStopped() | 9 |
| closeEvent 中录制停止 | 2 |
| **合计迁出** | **~322** |

### 6.2 新增行数 (委托调用替代)

| 新增内容 | 行数 |
|---------|------|
| NavigationController 构造 + buildNavTree 调用 | ~3 |
| NavigationController::buildNavPanelMappings 调用 | ~8 |
| RecordingController 构造 | ~1 |
| RecordingController::setupActions 调用 | ~1 |
| NavigationController 信号连接 | ~5 |
| RecordingController 信号连接 | ~12 |
| switchToPanel 调用改为 m_navController-> | ~3 处各 1 行 |
| closeEvent 委托 | ~1 |
| **合计新增** | **~35** |

### 6.3 最终预估

```
MainWindow.cpp 当前行数:        1092
减去迁出:                      -322
加上新增委托代码:               +35
加上头文件 include 新增:         +2
-----------------------------------
预估最终行数:                   ~807
```

**注意**: 仅拆分 NavigationController + RecordingController 不足以将 MainWindow.cpp 降到 500 行以下。
本次拆分后约 807 行, 还需要后续迭代继续拆分:

| 后续拆分目标 | 预计可再减少 | 达到 |
|-------------|-------------|------|
| 本轮 (NavigationController + RecordingController) | ~287 | ~807 行 |
| SendController (发送区域逻辑) | ~80 | ~727 行 |
| ConnectionController (连接管理逻辑) | ~120 | ~607 行 |
| 将 setupUI 中的面板创建提取到 PanelFactory | ~150 | ~457 行 |

最终目标 ~457 行, 满足 < 500 行约束。本次先完成前两个拆分。

---

## 七、构造顺序与生命周期

### 7.1 MainWindow 构造函数变更

```cpp
MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , m_connManager(new ConnectionManager(this))
    , m_terminalModel(new TerminalModel(this))
    , m_timedSender(new TimedSender(this))
    , m_sendHistory(new SendHistory(this))
    , m_dataExporter(new DataExporter(this))
    , m_dataLogger(new DataLogger(this))
    , m_statsTimer(new QTimer(this))
    , m_frameParser(new FrameParser(this))
    , m_otaManager(new OtaManager(this))
    , m_navController(new NavigationController(this))       // 新增
    , m_recordingController(new RecordingController(m_dataLogger, this))  // 新增
{
    setupUI();
    setupToolbar();
    setupStatusBar();

    // 导航面板映射: 传入所有面板指针
    m_navController->buildNavPanelMappings(
        m_serialConfig, m_terminal, m_dataStats,
        m_protocolView, m_frameEditor, m_chartWidget, m_otaWidget
    );
    m_navController->setCurrentPanel(m_terminal);

    connectSignals();
    loadSettings();

    m_statsTimer->setInterval(500);
    m_statsTimer->start();

    setWindowTitle(App::APP_NAME);
    resize(1200, 800);
    setMinimumSize(900, 600);
}
```

### 7.2 对象所有权

```
MainWindow (QObject parent)
  |
  +-- ConnectionManager*       (QObject, this)
  +-- TerminalModel*           (QObject, this)
  +-- DataLogger*              (QObject, this)
  +-- NavigationController*    (QObject, this)   <-- 新增
  +-- RecordingController*     (QObject, this)   <-- 新增
  +-- QTreeView* m_navTree     (QObject, this)
  +-- ... (其他 UI 组件)
```

两个新控制器都是 MainWindow 的子对象, 生命周期由 Qt 父子树管理, 无需手动 delete。

---

## 八、分层合规性检查

### 8.1 NavigationController 分层定位

```
表现层 (Presentation)
  |
  +-- NavigationController   <-- 正确
```

NavigationController 管理导航树 (QTreeView) 和面板切换动画 (QPropertyAnimation),
全部是表现层行为。它不直接操作业务对象 (ConnectionManager, DataLogger)。
功能性节点点击通过信号转发给 MainWindow, 由 MainWindow 编排业务动作。

### 8.2 RecordingController 分层定位

```
业务层 (Business)
  |
  +-- RecordingController     <-- 正确
```

RecordingController 编排录制/回放的业务流程:
- 选择文件路径 (QFileDialog) -- 此处涉及 UI, 但录制启停的决策逻辑是业务层
- 控制 DataLogger 的 start/stop/pause/resume
- 转发回放数据到终端 (通过信号, 不直接操作 TerminalModel)

注意: RecordingController 中有 QFileDialog 调用, 严格来说这是表现层行为。
但考虑到文件路径选择是录制流程不可分割的一部分, 且 RecordingController
通过 `m_parentWidget()` 获取父窗口用于模态对话框, 这种跨层耦合是可接受的。
如果未来需要完全解耦, 可以将文件选择提取为策略接口。

### 8.3 依赖方向检查

```
MainWindow (表现层)
  --> NavigationController (表现层)     正确: 同层引用
  --> RecordingController (业务层)      正确: 上层依赖下层
  --> DataLogger (数据层)               正确: 上层依赖下层

RecordingController (业务层)
  --> DataLogger (数据层)              正确: 上层依赖下层
  --X-- TerminalModel (表现层)          禁止: 通过信号解耦
  --X-- QWidget / statusBar            禁止: 通过信号解耦

NavigationController (表现层)
  --X-- ConnectionManager (业务层)     禁止: 通过信号解耦
  --X-- DataLogger (数据层)            无依赖
```

所有依赖方向均满足 CLAUDE.md 4.3 的单向规则。

---

## 九、CMakeLists.txt 变更

在 `src/core` 对应的 CMakeLists.txt 中新增两个源文件:

```cmake
# 新增源文件
src/core/NavigationController.h
src/core/NavigationController.cpp
src/core/RecordingController.h
src/core/RecordingController.cpp
```

---

## 十、验证清单

| 检查项 | 预期结果 |
|-------|---------|
| NavigationController 编译通过 | 零错误 |
| RecordingController 编译通过 | 零错误 |
| MainWindow.cpp 行数 < 500 (最终目标) | 本轮后 ~807 |
| 导航树点击切换面板 + 动画正常 | 行为不变 |
| 功能性节点 (数据导出/TCP/UDP) 正常触发 | 行为不变 |
| 录制/暂停/恢复/停止 功能正常 | 行为不变 |
| 回放日志文件 + 进度显示正常 | 行为不变 |
| 连接成功后自动切换到终端面板 | 行为不变 |
| closeEvent 停止录制/回放 | 行为不变 |
| EmbedDebug.bat 启动正常 | 行为不变 |
