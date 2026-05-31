# 架构设计文档 ARCH_DESIGN_007

> 设计人: 系统架构师 (system-architect)
> 设计日期: 2026-05-31
> 关联约束: CLAUDE.md 4.6 (MainWindow.cpp 必须 < 500 行)
> 涉及模块: core/MainWindow, core/SendController, serial/TimedSender, serial/SendHistory, serial/QuickCommandBar
> 预计变更: ~250 行新增 (一个新类), ~95 行从 MainWindow.cpp 迁出
> 目标: MainWindow.cpp 从 762 行降至 ~680 行 (本轮), 配合后续 ConnectionController 拆分达到 < 500 行

---

## 一、设计背景

MainWindow.cpp 当前 762 行 (经过 ARCH_DESIGN_006 的 NavigationController/RecordingController 拆分后)。
仍需进一步拆分以满足 CLAUDE.md 4.6 节 < 500 行的铁律。

发送相关逻辑分散在 MainWindow 的 4 个方法、6 个成员变量和 3 段信号连接中:

| 职责 | 涉及成员 | MainWindow.cpp 行号 |
|------|---------|---------------------|
| 发送输入区 UI 创建 | m_sendModeCombo, m_sendInput, m_sendBtn, m_sendCompleter, m_sendCompleterModel | 第 158-187 行 (setupUI 内) |
| 快捷指令栏 UI 创建 | m_quickCmdBar | 第 149-156 行 (setupUI 内) |
| 定时发送器构造 | m_timedSender | 第 23 行 (构造函数初始化列表) |
| 发送按钮信号连接 | m_sendBtn->clicked, m_sendInput->returnPressed | 第 283-284 行 (connectSignals 内) |
| 快捷指令信号连接 | m_quickCmdBar->commandTriggered | 第 287-288 行 (connectSignals 内) |
| 定时发送信号连接 | m_timedSender->sendData lambda | 第 331-333 行 (connectSignals 内) |
| 历史补全信号连接 | m_sendHistory->historyChanged lambda | 第 336-338 行 (connectSignals 内) |
| sendAndRecord 核心 | m_currentConn->write, m_terminalModel->appendSent, m_dataLogger->logData | 第 497-511 行 |
| onSendData 入口 | HEX 解析、输入校验、错误态样式 | 第 513-543 行 |
| onQuickCommand 入口 | 快捷指令转发 | 第 545-548 行 |

以上合计约 95 行实际代码。提取为独立的 SendController 类后:
- MainWindow 失去发送相关的所有 UI 创建、信号连接和业务编排代码
- SendController 封装发送输入框、HEX 转换、历史补全、定时发送、快捷指令的完整生命周期
- MainWindow 仅保留 "连接管理" 和 "面板布局" 两个核心职责

---

## 二、SendController 类图

```
                            QObject
                               |
                        SendController
                               |
          +--------------------+---------------------+-------------------+
          |                    |                     |                   |
     TerminalModel*       DataLogger*          SendHistory*        IConnection*
     (外部创建,           (外部创建,            (外部创建,          (动态注入,
      引用不拥有)          引用不拥有)           引用不拥有)         setConnection)

     自有子对象:
       TimedSender*           -- 定时发送器
       QuickCommandBar*       -- 快捷指令栏
       QLineEdit*             -- 发送输入框
       QPushButton*           -- 发送按钮
       QComboBox*             -- 文本/HEX 切换
       QCompleter*            -- 发送历史补全
       QStringListModel*      -- 补全数据模型

    signals:
      dataSent(qint64 bytes)
      statusMessage(const QString&)
      connectionRequired()              -- 发送时发现无连接, 通知 MainWindow

    public slots:
      onSendData()
      onQuickCommand(const QByteArray&)
      setConnection(IConnection*)       -- 连接变化时注入当前连接
```

---

## 三、头文件设计

```cpp
// src/core/SendController.h
#ifndef SEND_CONTROLLER_H
#define SEND_CONTROLLER_H

#include <QObject>
#include <QLineEdit>
#include <QPushButton>
#include <QComboBox>
#include <QStringListModel>
#include <QCompleter>

class TerminalModel;
class DataLogger;
class SendHistory;
class TimedSender;
class QuickCommandBar;
class IConnection;

// 发送控制器 - 封装所有发送相关的 UI 创建、输入处理、历史补全和定时发送
// 职责:
//   1. 创建和管理发送输入区域 (输入框 + HEX/文本切换 + 发送按钮)
//   2. 创建和管理快捷指令栏
//   3. HEX 输入校验和转换
//   4. 发送历史自动补全
//   5. 定时发送器的启停控制
//   6. 统一发送入口: 写入连接 + 记录终端 + 日志
class SendController : public QObject {
    Q_OBJECT

public:
    // 构造函数: 传入核心依赖 (均由外部拥有, 引用不持有)
    // parent 用于 QObject 父子树管理
    explicit SendController(TerminalModel* terminalModel,
                            DataLogger* dataLogger,
                            SendHistory* sendHistory,
                            QObject* parent = nullptr);
    ~SendController() override;

    // 创建发送输入区域 (输入框 + HEX切换 + 发送按钮) 并返回容器 Widget
    // 在 MainWindow::setupUI 中调用, 返回值直接 addLayout/addWidget 到目标布局
    QWidget* createSendInputArea();

    // 创建快捷指令栏并返回
    // 在 MainWindow::setupUI 中调用
    QuickCommandBar* createQuickCommandBar();

    // 连接变化时注入当前连接 (由 MainWindow::onConnectSerial 等调用)
    // 传 nullptr 表示断开连接
    void setConnection(IConnection* connection);

    // 获取快捷指令栏指针 (MainWindow 可能需要获取指针做布局)
    QuickCommandBar* quickCommandBar() const;

    // 停止定时发送器 (供 closeEvent 调用)
    void stopTimedSender();

    // 禁用拷贝
    SendController(const SendController&) = delete;
    SendController& operator=(const SendController&) = delete;

signals:
    // 数据已发送, 通知 MainWindow 更新状态栏
    void dataSent(qint64 bytes);

    // 状态栏消息 (如发送失败提示)
    void statusMessage(const QString& msg);

    // 发送时发现无连接, 通知 MainWindow (预留, 用于显示连接提示)
    void connectionRequired();

public slots:
    // 用户点击发送按钮或按回车
    void onSendData();

    // 快捷指令触发
    void onQuickCommand(const QByteArray& data);

private slots:
    // 定时发送器触发
    void onTimedSendData(const QByteArray& data);

    // 发送历史变化, 更新补全模型
    void onHistoryChanged();

private:
    // 统一发送方法: 写入连接 + 记录终端 + 日志, 返回是否成功
    bool sendAndRecord(const QByteArray& data, bool isHex = false);

    // 设置发送输入框的错误/正常样式
    void setInputError(bool error);

    // 核心依赖 (外部拥有, 引用不持有)
    TerminalModel* m_terminalModel;
    DataLogger* m_dataLogger;
    SendHistory* m_sendHistory;

    // 动态注入的连接 (由 MainWindow 通过 setConnection 设置)
    IConnection* m_currentConn = nullptr;

    // 自有子对象 (QObject 父子树管理生命周期)
    TimedSender* m_timedSender;
    QuickCommandBar* m_quickCmdBar = nullptr;

    // 发送区域 UI 组件
    QLineEdit* m_sendInput = nullptr;
    QPushButton* m_sendBtn = nullptr;
    QComboBox* m_sendModeCombo = nullptr;
    QCompleter* m_sendCompleter = nullptr;
    QStringListModel* m_sendCompleterModel = nullptr;
};

#endif // SEND_CONTROLLER_H
```

---

## 四、方法签名与职责

| 方法 | 来源 (MainWindow.cpp 行号) | 职责 |
|------|---------------------------|------|
| `SendController(TerminalModel*, DataLogger*, SendHistory*, QObject*)` | -- | 构造, 保存依赖引用, 创建 TimedSender |
| `createSendInputArea()` | 第 158-187 行 (setupUI 内) | 创建 QFrame 容器, 内含 m_sendModeCombo + m_sendInput + m_sendBtn, 配置补全器, 返回容器 |
| `createQuickCommandBar()` | 第 149-156 行 (setupUI 内) | 创建 QuickCommandBar, 设置默认指令, 返回指针 |
| `setConnection(IConnection*)` | m_currentConn 赋值 (分散) | 注入/清除当前连接, 由 MainWindow 在连接/断开时调用 |
| `quickCommandBar()` | -- | getter, 供 MainWindow 布局使用 |
| `stopTimedSender()` | closeEvent 中可能需要 | 停止定时发送 |
| `onSendData()` | 第 513-543 行 | 读取输入, HEX 转换, 调用 sendAndRecord, 更新历史, 清空输入框 |
| `onQuickCommand(const QByteArray&)` | 第 545-548 行 | 转发到 sendAndRecord |
| `onTimedSendData(const QByteArray&)` | 第 331-333 行 (lambda) | 转发到 sendAndRecord |
| `onHistoryChanged()` | 第 336-338 行 (lambda) | 更新补全模型 |
| `sendAndRecord(const QByteArray&, bool)` | 第 497-511 行 | 写入连接, 记录终端, 记录日志, 发 dataSent 信号 |
| `setInputError(bool)` | 第 528-530, 539-541 行 | 统一设置输入框错误/正常样式 |

---

## 五、信号/槽连接

### 5.1 SendController 内部连接

在 `createSendInputArea()` 中建立:

```
m_sendBtn->clicked              -->  onSendData()
m_sendInput->returnPressed      -->  onSendData()
m_sendHistory->historyChanged   -->  onHistoryChanged()
m_timedSender->sendData         -->  onTimedSendData(QByteArray)
```

在 `createQuickCommandBar()` 中建立:

```
m_quickCmdBar->commandTriggered -->  onQuickCommand(QByteArray)
```

### 5.2 SendController 发出的信号 --> MainWindow 接收

```
SendController::dataSent(qint64)         -->  MainWindow::updateStatusBar()
SendController::statusMessage(QString)   -->  statusBar()->showMessage(msg, 3000)
SendController::connectionRequired()     -->  (预留, 可用于弹窗提示连接)
```

### 5.3 MainWindow --> SendController 的调用

```
MainWindow::onConnectSerial()            -->  m_sendController->setConnection(m_currentConn)
MainWindow::onDisconnectSerial()         -->  m_sendController->setConnection(nullptr)
MainWindow::onConnectNetwork()           -->  m_sendController->setConnection(m_currentConn)
MainWindow::onConnectionStateChanged()   -->  (连接失败时) m_sendController->setConnection(nullptr)
MainWindow::closeEvent()                 -->  m_sendController->stopTimedSender()
```

---

## 六、MainWindow 重构后的结构

### 6.1 从 MainWindow.h 中移除的成员

```cpp
// 以下成员从 MainWindow.h 中删除:
QLineEdit* m_sendInput;
QPushButton* m_sendBtn;
QComboBox* m_sendModeCombo;
QCompleter* m_sendCompleter;
QStringListModel* m_sendCompleterModel;
QuickCommandBar* m_quickCmdBar;
TimedSender* m_timedSender;
```

### 6.2 从 MainWindow.h 中新增的成员

```cpp
// 新增成员:
SendController* m_sendController;
```

### 6.3 从 MainWindow.h 中移除的方法

```cpp
// 以下 slots 从 MainWindow.h 中删除:
void onSendData();
void onQuickCommand(const QByteArray& data);

// 以下 private 方法从 MainWindow.h 中删除:
bool sendAndRecord(const QByteArray& data, bool isHex = false);
```

### 6.4 保留在 MainWindow 中的方法

MainWindow 保留的职责仅剩:
- setupUI() -- 创建布局框架, 调用 m_sendController->createSendInputArea() / createQuickCommandBar()
- setupToolbar() -- 工具栏
- setupStatusBar() -- 状态栏
- connectSignals() -- 信号编排 (减少约 10 行发送相关连接)
- updateStatusBar() -- RX/TX 刷新
- updateDataStatistics() -- 统计刷新
- loadSettings() / saveSettings() -- 设置
- onConnectSerial() / onDisconnectSerial() / onConnectNetwork() -- 连接管理 (注入 m_sendController->setConnection)
- onDisplayModeChanged() / onTimestampToggled() / onClearTerminal()
- onExportData()
- onSearchRequested() / onSearchCleared()
- onThemeChanged() / onLanguageChanged()
- onConnectionStateChanged()
- onDataReceived() -- 接收数据
- closeEvent()

---

## 七、逐行迁移计划

### 7.1 MainWindow.cpp 迁出明细

| MainWindow.cpp 行号 | 内容 | 迁移目标 | 动作 |
|---------------------|------|---------|------|
| 23 | `m_timedSender(new TimedSender(this))` | SendController 构造函数 | 删除, SendController 内部创建 |
| 149-156 | 快捷指令栏创建 (setupUI 内) | SendController::createQuickCommandBar() | 提取为方法调用 |
| 158-187 | 发送输入区创建 (setupUI 内) | SendController::createSendInputArea() | 提取为方法调用 |
| 283-288 | 发送按钮/快捷指令信号连接 (connectSignals 内) | SendController::createSendInputArea() 内部 | 删除, SendController 自建 |
| 331-333 | 定时发送信号连接 (connectSignals 内) | SendController 内部连接 | 删除, SendController 自建 |
| 336-338 | 历史补全信号连接 (connectSignals 内) | SendController 内部连接 | 删除, SendController 自建 |
| 497-511 | `sendAndRecord()` | SendController::sendAndRecord() | 移动 |
| 513-543 | `onSendData()` | SendController::onSendData() | 移动 |
| 545-548 | `onQuickCommand()` | SendController::onQuickCommand() | 移动 |

### 7.2 MainWindow.cpp 替换为委托调用

setupUI() 中:

```cpp
// 原代码 (第 149-187 行, 约 39 行):
// m_quickCmdBar = new QuickCommandBar;
// ... 设置指令 ...
// auto* sendFrame = new QFrame;
// ... 创建输入框、补全器、按钮 ...
// serialLayout->addWidget(sendFrame);

// 替换为 (约 4 行):
m_quickCmdBar = m_sendController->createQuickCommandBar();
serialLayout->addWidget(m_quickCmdBar);
serialLayout->addWidget(m_sendController->createSendInputArea());
```

connectSignals() 中:

```cpp
// 原代码 (第 283-288, 331-338 行, 约 14 行):
// connect(m_sendBtn, ...);
// connect(m_sendInput, ...);
// connect(m_quickCmdBar, ...);
// connect(m_timedSender, ...);
// connect(m_sendHistory, ...);

// 替换为 (约 2 行):
connect(m_sendController, &SendController::dataSent, this, &MainWindow::updateStatusBar);
connect(m_sendController, &SendController::statusMessage, this, [this](const QString& msg) {
    statusBar()->showMessage(msg, 3000);
});
```

onConnectSerial() 中新增:

```cpp
// 连接成功后注入连接:
m_sendController->setConnection(m_currentConn);
```

onDisconnectSerial() 中新增:

```cpp
m_sendController->setConnection(nullptr);
```

onConnectNetwork() 中新增:

```cpp
// 连接成功后注入:
m_sendController->setConnection(m_currentConn);
```

closeEvent() 中新增:

```cpp
m_sendController->stopTimedSender();
```

---

## 八、预期行数分析

### 8.1 迁出行数统计

| 迁出内容 | 行数 |
|---------|------|
| 快捷指令栏创建 (setupUI 内) | 8 |
| 发送输入区创建 (setupUI 内) | 30 |
| 发送按钮/回车信号连接 | 6 |
| 快捷指令信号连接 | 3 |
| 定时发送信号连接 | 3 |
| 历史补全信号连接 | 3 |
| sendAndRecord() 方法 | 15 |
| onSendData() 方法 | 31 |
| onQuickCommand() 方法 | 4 |
| m_timedSender 初始化列表项 | 1 |
| **合计迁出** | **~104** |

### 8.2 新增行数 (委托调用替代)

| 新增内容 | 行数 |
|---------|------|
| m_sendController 构造 (初始化列表) | 1 |
| createQuickCommandBar() 调用 + addWidget | 2 |
| createSendInputArea() 调用 + addWidget | 2 |
| SendController 信号连接 (dataSent, statusMessage) | 4 |
| onConnectSerial 中 setConnection 调用 | 1 |
| onDisconnectSerial 中 setConnection 调用 | 1 |
| onConnectNetwork 中 setConnection 调用 | 1 |
| closeEvent 中 stopTimedSender 调用 | 1 |
| #include "core/SendController.h" | 1 |
| **合计新增** | **~14** |

### 8.3 SendController.cpp 预计行数

| 内容 | 行数 |
|------|------|
| 构造函数 (创建 TimedSender, 无需内部连接 -- 在 create 方法中连) | 15 |
| createSendInputArea() (创建 UI 组件 + 补全器 + 内部信号连接) | 45 |
| createQuickCommandBar() (创建 + 设置指令 + 内部信号连接) | 15 |
| setConnection() | 5 |
| quickCommandBar() | 3 |
| stopTimedSender() | 5 |
| onSendData() (从 MainWindow 迁入) | 35 |
| onQuickCommand() (从 MainWindow 迁入) | 5 |
| onTimedSendData() (从 lambda 展开) | 5 |
| onHistoryChanged() (从 lambda 展开) | 5 |
| sendAndRecord() (从 MainWindow 迁入) | 20 |
| setInputError() (从 onSendData 中提取) | 8 |
| 头文件 + include | 10 |
| **合计** | **~176** |

### 8.4 最终预估

```
MainWindow.cpp 当前行数:          762
减去迁出:                       -104
加上新增委托代码:                +14
-----------------------------------
预估 MainWindow.cpp 行数:        ~672
```

本轮拆分后 MainWindow.cpp 约 672 行, 仍需后续迭代继续拆分:

| 后续拆分目标 | 预计可再减少 | 达到 |
|-------------|-------------|------|
| 本轮 (SendController) | ~90 | ~672 行 |
| ConnectionController (连接管理逻辑) | ~100 | ~572 行 |
| 将 setupUI 中的面板创建提取到 PanelFactory | ~120 | ~452 行 |

最终目标 ~452 行, 满足 < 500 行约束。

---

## 九、构造顺序与生命周期

### 9.1 MainWindow 构造函数变更

```cpp
MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , m_connManager(new ConnectionManager(this))
    , m_terminalModel(new TerminalModel(this))
    , m_sendHistory(new SendHistory(this))
    , m_dataExporter(new DataExporter(this))
    , m_dataLogger(new DataLogger(this))
    , m_recordingController(new RecordingController(m_dataLogger, this))
    , m_sendController(new SendController(m_terminalModel, m_dataLogger, m_sendHistory, this))  // 新增
    , m_statsTimer(new QTimer(this))
    , m_frameParser(new FrameParser(this))
    , m_otaManager(new OtaManager(this))
    , m_navController(new NavigationController(this))
{
    setupUI();
    setupToolbar();
    setupStatusBar();
    connectSignals();

    m_navController->buildNavTree(m_navTree, { /* ... */ });
    m_navController->setCurrentPanel(m_terminal);

    loadSettings();

    m_statsTimer->setInterval(500);
    m_statsTimer->start();

    setWindowTitle(App::APP_NAME);
    resize(1200, 800);
    setMinimumSize(900, 600);
}
```

注意: `m_timedSender` 不再出现在 MainWindow 构造函数中, 改为 SendController 内部创建。

### 9.2 对象所有权

```
MainWindow (QObject parent)
  |
  +-- ConnectionManager*            (QObject, this)
  +-- TerminalModel*                (QObject, this)
  +-- SendHistory*                  (QObject, this)
  +-- DataLogger*                   (QObject, this)
  +-- NavigationController*         (QObject, this)
  +-- RecordingController*          (QObject, this)
  +-- SendController*               (QObject, this)   <-- 新增
  |     |
  |     +-- TimedSender*            (QObject, m_sendController)
  |     +-- QuickCommandBar*        (QObject, m_sendController)
  |     +-- QLineEdit* m_sendInput  (QObject, sendFrame -> m_sendController)
  |     +-- ... (发送区 UI 组件)
  |
  +-- QTreeView* m_navTree          (QObject, this)
  +-- ... (其他 UI 组件)
```

SendController 是 MainWindow 的子对象, 生命周期由 Qt 父子树管理。
SendController 内部创建的 UI 组件以 SendController 为 parent (或其子 Widget 为 parent)。

---

## 十、分层合规性检查

### 10.1 SendController 分层定位

```
业务层 (Business)
  |
  +-- SendController              <-- 正确
```

SendController 编排发送流程:
- 读取用户输入, 执行 HEX 转换 (依赖 HexConverter -- 基础设施层)
- 写入连接 (依赖 IConnection -- 基础设施层)
- 记录终端 (依赖 TerminalModel -- 数据层)
- 记录日志 (依赖 DataLogger -- 数据层)
- 管理历史 (依赖 SendHistory -- 数据层)
- 管理定时发送 (自有 TimedSender -- 业务层)

注意: SendController 创建 UI 组件 (QLineEdit, QPushButton 等), 严格来说这是表现层行为。
但发送输入区与发送业务逻辑高度耦合 (输入校验、HEX 转换、错误态显示), 将 UI 创建与业务逻辑
放在同一个控制器中是合理的, 类似于 RecordingController 中 QFileDialog 的处理方式。
这避免了将 "输入框错误态" 这类 UI 细节暴露给 MainWindow。

### 10.2 依赖方向检查

```
SendController (业务层)
  --> TerminalModel (数据层)            正确: 上层依赖下层
  --> DataLogger (数据层)              正确: 上层依赖下层
  --> SendHistory (数据层)             正确: 上层依赖下层
  --> IConnection (基础设施层)          正确: 上层依赖下层
  --> HexConverter (基础设施层)         正确: 上层依赖下层
  --> TimedSender (业务层)             正确: 同层引用
  --X-- MainWindow (表现层)             禁止: 通过信号解耦

MainWindow (表现层)
  --> SendController (业务层)           正确: 上层依赖下层
```

所有依赖方向均满足 CLAUDE.md 4.3 的单向规则。

---

## 十一、CMakeLists.txt 变更

在 `src/core` 对应的 CMakeLists.txt 中新增:

```cmake
# 新增源文件
src/core/SendController.h
src/core/SendController.cpp
```

---

## 十二、验证清单

| 检查项 | 预期结果 |
|-------|---------|
| SendController 编译通过 | 零错误 |
| MainWindow.cpp 行数 | ~672 (本轮) |
| 文本模式发送: 输入文本, 点击发送, 数据写入连接 | 行为不变 |
| HEX 模式发送: 输入 "AA BB CC", 数据正确转换并发送 | 行为不变 |
| HEX 输入错误: 输入非法 HEX 字符, 输入框显示红色错误态 | 行为不变 |
| 发送历史: 发送后历史记录更新, 下拉补全可用 | 行为不变 |
| 回车发送: 在输入框按回车等同于点击发送按钮 | 行为不变 |
| 快捷指令: 点击 AT/Reset/Status 按钮, 数据正确发送 | 行为不变 |
| 定时发送: 定时器触发, 数据自动发送 | 行为不变 |
| 无连接时发送: 静默忽略 (不崩溃) | 行为不变 |
| 发送后清空输入框 | 行为不变 |
| 连接/断开时 setConnection 正确注入/清除 | 行为不变 |
| closeEvent 停止定时发送 | 行为不变 |
| EmbedDebug.bat 启动正常 | 行为不变 |
