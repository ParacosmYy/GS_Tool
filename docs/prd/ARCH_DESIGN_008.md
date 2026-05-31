# 架构设计文档 ARCH_DESIGN_008

> 设计人: 系统架构师 (system-architect)
> 设计日期: 2026-05-31
> 关联约束: CLAUDE.md 4.6 (MainWindow.cpp 必须 < 500 行)
> 涉及模块: core/MainWindow, core/ConnectionController (新增)
> 前置依赖: ARCH_DESIGN_007 (SendController 提取已完成)
> 预计变更: ~220 行新增 (一个新类), ~175 行从 MainWindow.cpp 迁出
> 目标: MainWindow.cpp 从 682 行降至 ~450 行 (满足 < 500 行铁律)

---

## 一、设计背景

MainWindow.cpp 当前 682 行 (经过 SendController 提取后的 commit #22 状态)。
经过 ARCH_DESIGN_005/006/007 三轮拆分 (NavigationController 265 行, RecordingController 136 行, SendController 146 行),
MainWindow 仍承载大量连接管理逻辑。这是将 MainWindow 压到 500 行以下的关键步骤。

连接相关逻辑分散在 MainWindow 的 6 个方法、6 个成员变量和多段信号连接中:

| 职责 | 涉及成员 | MainWindow.cpp 行号 | 行数 |
|------|---------|---------------------|------|
| 串口连接创建+配置+打开 | m_connManager, m_currentConn, m_serialConfig | 第 411-454 行 | 44 |
| 串口断开+清理 | m_currentConn, m_connManager, m_sendController | 第 456-464 行 | 9 |
| 网络连接创建+配置+打开 | m_connManager, m_currentConn | 第 550-593 行 | 44 |
| 连接状态变化处理 | m_connStatusLbl, m_serialConfig, m_recordingController, m_navController | 第 595-632 行 | 38 |
| 接收数据分发 | m_terminalModel, m_frameParser, m_dataLogger | 第 634-640 行 | 7 |
| 状态栏字节统计 | m_rxBytesLbl, m_txBytesLbl, m_terminalModel | 第 642-655 行 | 14 |
| closeEvent 中连接关闭 | m_connManager | 第 677-680 行 | 4 |
| connectSignals 中串口信号连接 | m_serialConfig | 第 250-253 行 | 4 |
| connectSignals 中导航树网络节点 | onConnectNetwork 调用 | 第 333-335 行 | 3 |
| m_connManager / m_currentConn 初始化 | 构造函数初始化列表 | 第 21, 98 行 | 2 |
| 成员声明 | m_connManager, m_currentConn, m_connStatusLbl, m_rxBytesLbl, m_txBytesLbl | MainWindow.h | 5 |

以上合计约 174 行实际代码。提取为独立的 ConnectionController 类后:
- MainWindow 失去连接创建、断开、状态同步、数据分发、状态栏刷新的全部逻辑
- ConnectionController 封装从 IConnection 创建到数据分发的完整连接生命周期
- MainWindow 仅保留 "UI 布局框架" 和 "设置/主题/语言" 等纯 UI 职责

---

## 二、设计挑战与约束分析

### 2.1 不直接操作 QLabel UI 控件

连接状态变化时需要更新三个 QLabel (m_connStatusLbl, m_rxBytesLbl, m_txBytesLbl),
还需要修改 QLabel 的 property 和 style (polish/unpolish), 以及控制呼吸动画。
这些操作直接操作 QWidget, 但 ConnectionController 属于业务层, 不能持有 UI 控件引用。

**解决方案**: ConnectionController 通过信号通知 MainWindow 进行 UI 更新。
状态变化时发出 `connectionStateChanged(ConnectionState, const QString&)` 信号,
附带连接名称字符串, 由 MainWindow 在槽函数中更新 QLabel。

### 2.2 数据分发需要多个下游消费者

`onDataReceived()` 将数据分发到三个消费者:
1. TerminalModel::appendReceived() -- 数据层
2. FrameParser::feed() -- 数据层
3. DataLogger::logData() -- 数据层

ConnectionController 位于业务层, 可以直接依赖数据层组件。
将这三个依赖通过构造函数注入, 由 ConnectionController 内部直接调用,
无需通过信号中转, 减少开销。

### 2.3 连接状态变化需要同步多个组件

`onConnectionStateChanged()` 需要通知:
1. SerialConfigPanel::setConnected() -- 表现层
2. RecordingController::setConnected() -- 业务层
3. NavigationController -- 呼吸动画和面板切换 -- 表现层
4. SendController/OtaManager::setConnection() -- 业务层

表现层组件 (SerialConfigPanel, NavigationController) 不能被业务层直接依赖。
**解决方案**: 对表现层组件的更新通过信号通知 MainWindow 执行;
对业务层组件 (RecordingController, SendController, OtaManager) 的调用,
由 ConnectionController 在内部直接执行 (业务层内部通信)。

### 2.4 QMessageBox 弹窗归属

onConnectSerial/onConnectNetwork 中有 QMessageBox::warning 调用。
弹窗是表现层行为, 但连接失败通知是业务事件。
**解决方案**: 连接失败时发出信号 `connectionFailed(const QString&)`,
由 MainWindow 决定是否弹窗显示。这是更松耦合的设计,
未来可以替换为 Toast 通知而无需修改 ConnectionController。

---

## 三、ConnectionController 类图

```
                              QObject
                                 |
                        ConnectionController
                                 |
        +------------+-----------+-----------+------------+------------+
        |            |           |           |            |            |
   ConnectionManager  TerminalModel   FrameParser   DataLogger    IConnection*
   (外部创建,        (外部创建,      (外部创建,    (外部创建,    (动态创建/销毁,
    引用不拥有)       引用不拥有)     引用不拥有)   引用不拥有)   内部管理生命周期)

   注入的目标 (业务层内部, 直接调用):
     SendController*        -- setConnection()
     OtaManager*            -- setConnection()
     RecordingController*   -- setConnected()

   signals:
     connectionStateChanged(ConnectionState, const QString& name)
     connectionFailed(const QString& errorMsg)
     statusMessage(const QString& msg)
     statusBarUpdateRequested()
     navigateToTerminalRequested()
     serialConfigConnectedChanged(bool connected)
     breathingAnimationRequested(bool start)
```

---

## 四、头文件设计

```cpp
// src/core/ConnectionController.h
#ifndef CONNECTION_CONTROLLER_H
#define CONNECTION_CONTROLLER_H

#include <QObject>
#include "core/Constants.h"

class ConnectionManager;
class TerminalModel;
class FrameParser;
class DataLogger;
class SendController;
class OtaManager;
class RecordingController;
class IConnection;

// 连接控制器 - 封装所有连接生命周期管理逻辑
// 职责:
//   1. 创建和配置连接 (串口/网络)
//   2. 管理连接信号连接和数据分发
//   3. 协调连接状态变化时各模块的同步
//   4. 向 SendController/OtaManager 注入 IConnection*
//   5. 更新 RX/TX 字节统计
//   6. 通知 MainWindow 进行 UI 状态更新 (通过信号)
class ConnectionController : public QObject {
    Q_OBJECT

public:
    explicit ConnectionController(ConnectionManager* connManager,
                                  TerminalModel* terminalModel,
                                  FrameParser* frameParser,
                                  DataLogger* dataLogger,
                                  QObject* parent = nullptr);
    ~ConnectionController() override;

    // 设置业务层协作对象 (在 MainWindow 构造后调用, 因为存在构造顺序依赖)
    void setSendController(SendController* controller);
    void setOtaManager(OtaManager* manager);
    void setRecordingController(RecordingController* controller);

    // 串口连接: 创建 + 配置 + 打开
    // configProvider 由 MainWindow 提供, 从 SerialConfigPanel 读取当前配置
    using SerialConfigProvider = std::function<QVariantMap()>;
    void connectSerial(const SerialConfigProvider& configProvider);

    // 串口断开: 关闭 + 清理
    void disconnectSerial();

    // 网络连接: 创建 + 配置 + 打开
    void connectNetwork(ConnectionType type);

    // 获取当前活动连接 (仅供外部只读查询, 如 saveSettings)
    IConnection* currentConnection() const;

    // 关闭所有连接 (供 closeEvent 调用)
    void closeAllConnections();

    // 刷新状态栏 RX/TX 字节统计 (由 statsTimer 或外部事件触发)
    void updateByteCounters();

    // 禁用拷贝
    ConnectionController(const ConnectionController&) = delete;
    ConnectionController& operator=(const ConnectionController&) = delete;

signals:
    // 连接状态变化: 通知 MainWindow 更新 UI (状态标签文本、property、样式)
    // name 参数为连接名称 (如 "COM3"), 用于显示在状态标签中
    void connectionStateChanged(ConnectionState state, const QString& name);

    // 连接失败: 通知 MainWindow 显示错误提示 (弹窗或 Toast)
    void connectionFailed(const QString& errorMsg);

    // 状态栏消息 (替代直接操作 statusBar)
    void statusMessage(const QString& msg, int timeoutMs = 0);

    // 状态栏 RX/TX 字节统计更新: 通知 MainWindow 更新 QLabel
    // rxText, txText 为格式化后的字符串 (如 "RX: 1.2 KB")
    void byteCountersUpdated(const QString& rxText, const QString& txText);

    // 连接成功后自动切换到终端面板
    void navigateToTerminalRequested();

    // SerialConfigPanel 连接状态变化
    void serialConfigConnectedChanged(bool connected);

    // 呼吸动画控制请求
    void breathingAnimationRequested(bool start);

private slots:
    // IConnection::stateChanged 信号的内部处理
    void onConnectionStateChanged(ConnectionState state);

    // IConnection::dataReceived 信号的内部处理
    void onDataReceived(const QByteArray& data);

    // IConnection::errorOccurred 信号的内部处理
    void onConnectionError(const QString& msg);

private:
    // 建立新连接的信号连接 (dataReceived/stateChanged/errorOccurred)
    void connectConnectionSignals(IConnection* conn);

    // 格式化字节数为可读字符串 (如 "1.2 KB")
    static QString formatBytes(quint64 bytes);

    // 核心依赖 (外部拥有, 引用不持有)
    ConnectionManager* m_connManager;
    TerminalModel* m_terminalModel;
    FrameParser* m_frameParser;
    DataLogger* m_dataLogger;

    // 业务层协作对象 (外部拥有, 引用不持有)
    SendController* m_sendController = nullptr;
    OtaManager* m_otaManager = nullptr;
    RecordingController* m_recordingController = nullptr;

    // 当前活动连接 (由 ConnectionController 管理生命周期)
    IConnection* m_currentConn = nullptr;
};

#endif // CONNECTION_CONTROLLER_H
```

---

## 五、方法签名与职责

| 方法 | 来源 (MainWindow.cpp 行号) | 职责 |
|------|---------------------------|------|
| `ConnectionController(...)` | -- | 构造, 保存依赖引用 |
| `setSendController(SendController*)` | -- | 注入 SendController (构造后调用) |
| `setOtaManager(OtaManager*)` | -- | 注入 OtaManager (构造后调用) |
| `setRecordingController(RecordingController*)` | -- | 注入 RecordingController (构造后调用) |
| `connectSerial(SerialConfigProvider)` | 第 411-454 行 | 创建串口连接, 通过 configProvider 读取配置, 打开连接, 注入 SendController/OtaManager |
| `disconnectSerial()` | 第 456-464 行 | 关闭连接, 清理资源, 通知 SendController 断开 |
| `connectNetwork(ConnectionType)` | 第 550-593 行 | 创建网络连接, 默认参数配置, 打开连接, 注入 SendController |
| `currentConnection()` | m_currentConn getter | 供外部只读查询 (如 saveSettings) |
| `closeAllConnections()` | 第 677-680 行 (closeEvent 内) | 遍历 ConnectionManager 关闭所有连接 |
| `updateByteCounters()` | 第 642-655 行 | 读取 rxBytes/txBytes, 格式化, 发 byteCountersUpdated 信号 |
| `onConnectionStateChanged(ConnectionState)` | 第 595-632 行 | 内部处理状态变化: 同步 RecordingController/SendController, 发 UI 通知信号 |
| `onDataReceived(const QByteArray&)` | 第 634-640 行 | 分发数据到 TerminalModel/FrameParser/DataLogger |
| `onConnectionError(const QString&)` | 第 438-440, 579-581 行 | 日志输出连接错误 |
| `connectConnectionSignals(IConnection*)` | 第 433-440, 574-581 行 (重复) | 统一的信号连接方法, 消除重复代码 |
| `formatBytes(quint64)` | 第 647-651 行 (lambda) | 静态工具方法, 字节数格式化 |

---

## 六、信号/槽连接

### 6.1 ConnectionController 内部连接

在 `connectConnectionSignals()` 中建立 (每次创建新连接时调用):

```
IConnection::dataReceived     -->  ConnectionController::onDataReceived(QByteArray)
IConnection::stateChanged     -->  ConnectionController::onConnectionStateChanged(ConnectionState)
IConnection::errorOccurred    -->  ConnectionController::onConnectionError(QString)
```

### 6.2 ConnectionController 发出的信号 --> MainWindow 接收

```
ConnectionController::connectionStateChanged(state, name)
    --> MainWindow::onConnectionStateChangedUI(state, name)    // 更新 QLabel 文本/property/样式

ConnectionController::connectionFailed(errorMsg)
    --> MainWindow::onConnectionFailedUI(errorMsg)             // 弹出 QMessageBox::warning

ConnectionController::statusMessage(msg, timeoutMs)
    --> statusBar()->showMessage(msg, timeoutMs)               // 状态栏消息

ConnectionController::byteCountersUpdated(rxText, txText)
    --> MainWindow::onByteCountersUpdated(rxText, txText)      // 更新 m_rxBytesLbl / m_txBytesLbl

ConnectionController::navigateToTerminalRequested()
    --> m_navController->switchToPanel(m_terminal)              // 切换到终端面板

ConnectionController::serialConfigConnectedChanged(connected)
    --> m_serialConfig->setConnected(connected)                 // 更新配置面板状态

ConnectionController::breathingAnimationRequested(start)
    --> MainWindow::onBreathingAnimation(start)                 // 启动/停止呼吸动画
```

### 6.3 MainWindow --> ConnectionController 的调用

```
SerialConfigPanel::connectRequested     -->  m_connController->connectSerial(configProvider)
SerialConfigPanel::disconnectRequested  -->  m_connController->disconnectSerial()
导航树 "TCP客户端" 点击                  -->  m_connController->connectNetwork(TcpClient)
导航树 "TCP服务端" 点击                  -->  m_connController->connectNetwork(TcpServer)
导航树 "UDP" 点击                        -->  m_connController->connectNetwork(Udp)
SendController::dataSent                -->  m_connController->updateByteCounters()
RecordingController::playbackData       -->  m_connController->updateByteCounters()
onClearTerminal()                       -->  m_connController->updateByteCounters()
closeEvent()                            -->  m_connController->closeAllConnections()
```

---

## 七、依赖关系图

### 7.1 模块依赖

```
                     表现层
                 ┌──────────────┐
                 │  MainWindow  │
                 └──────┬───────┘
                        │ 信号/槽 + 方法调用
                        v
               ┌────────────────────┐
               │ConnectionController│ 业务层
               └──┬─────┬─────┬────┘
                  │     │     │
        ┌─────────┘     │     └──────────┐
        v               v                v
  ┌───────────┐  ┌───────────┐   ┌─────────────┐
  │SendCtrl   │  │OtaManager │   │RecordingCtrl│  业务层 (同层协作)
  │.setConn() │  │.setConn() │   │.setConnected│
  └───────────┘  └───────────┘   └─────────────┘
        │               │
        v               v
  ┌───────────┐  ┌───────────┐
  │TerminalModel│ │FrameParser│        数据层
  │DataLogger  │ │           │
  └─────┬─────┘  └───────────┘
        │
        v
  ┌─────────────────┐
  │ConnectionManager│             基础设施层
  │IConnection      │
  └─────────────────┘
```

### 7.2 数据流

```
[用户点击连接] ──> SerialConfigPanel::connectRequested
                         │
                         v
              MainWindow ──> ConnectionController::connectSerial()
                         │
                         ├──> ConnectionManager::createConnection()
                         ├──> IConnection::configure(params)
                         ├──> IConnection::open()
                         ├──> SendController::setConnection(conn)
                         └──> OtaManager::setConnection(conn)

[IConnection 收到数据] ──> ConnectionController::onDataReceived()
                         │
                         ├──> TerminalModel::appendReceived()
                         ├──> FrameParser::feed()
                         └──> DataLogger::logData()

[连接状态变化] ──> ConnectionController::onConnectionStateChanged()
                         │
                         ├──> RecordingController::setConnected()
                         ├──> emit connectionStateChanged(state, name)  ──> MainWindow 更新 UI
                         ├──> emit breathingAnimationRequested(start)   ──> MainWindow 控制动画
                         ├──> emit serialConfigConnectedChanged(bool)   ──> MainWindow 更新配置面板
                         └──> emit navigateToTerminalRequested()        ──> MainWindow 切换面板
```

### 7.3 分层合规性检查

```
ConnectionController (业务层)
  --> ConnectionManager (基础设施层)      正确: 上层依赖下层
  --> TerminalModel (数据层)              正确: 上层依赖下层
  --> FrameParser (数据层)                正确: 上层依赖下层
  --> DataLogger (数据层)                正确: 上层依赖下层
  --> IConnection (基础设施层)            正确: 上层依赖下层
  --> SendController (业务层)            正确: 同层协作
  --> OtaManager (业务层)                正确: 同层协作
  --> RecordingController (业务层)       正确: 同层协作
  --X-- QLabel / QWidget (表现层)         禁止: 通过信号解耦
  --X-- SerialConfigPanel (表现层)        禁止: 通过信号解耦
  --X-- NavigationController (表现层)     禁止: 通过信号解耦
  --X-- MainWindow (表现层)               禁止: 通过信号解耦

MainWindow (表现层)
  --> ConnectionController (业务层)       正确: 上层依赖下层
```

所有依赖方向均满足 CLAUDE.md 4.3 的单向规则。

---

## 八、MainWindow 重构后的结构

### 8.1 从 MainWindow.h 中移除的成员

```cpp
// 以下成员从 MainWindow.h 中删除:
ConnectionManager* m_connManager;          // 移入 ConnectionController
IConnection* m_currentConn = nullptr;      // 移入 ConnectionController
```

### 8.2 从 MainWindow.h 中新增的成员

```cpp
// 新增成员:
ConnectionController* m_connController;    // 连接控制器
```

### 8.3 从 MainWindow.h 中保留但改变用途的成员

```cpp
// 以下成员仍在 MainWindow.h 中 (纯 UI 控件):
QLabel* m_connStatusLbl;                   // 保留, 仅由 MainWindow 操作
QLabel* m_rxBytesLbl;                      // 保留, 仅由 MainWindow 操作
QLabel* m_txBytesLbl;                      // 保留, 仅由 MainWindow 操作
```

### 8.4 从 MainWindow.h 中移除的 slots

```cpp
// 以下 slots 从 MainWindow.h 中删除:
void onConnectSerial();                    // 移入 ConnectionController::connectSerial()
void onDisconnectSerial();                 // 移入 ConnectionController::disconnectSerial()
void onConnectNetwork(ConnectionType);     // 移入 ConnectionController::connectNetwork()
void onConnectionStateChanged(ConnectionState); // 移入 ConnectionController
void onDataReceived(const QByteArray&);    // 移入 ConnectionController
```

### 8.5 从 MainWindow.h 中移除的 private 方法

```cpp
// 以下 private 方法从 MainWindow.h 中删除:
void updateStatusBar();                    // 移入 ConnectionController::updateByteCounters()
```

### 8.6 MainWindow.h 中新增的 private slots

```cpp
// 新增 (纯 UI 更新槽函数, 响应 ConnectionController 信号):
void onConnectionStateChangedUI(ConnectionState state, const QString& name);
void onConnectionFailedUI(const QString& errorMsg);
void onByteCountersUpdated(const QString& rxText, const QString& txText);
void onBreathingAnimationRequested(bool start);
```

### 8.7 保留在 MainWindow 中的方法

MainWindow 保留的职责仅为 "UI 布局框架" 和 "设置/主题/语言":

- setupUI() -- 创建布局框架
- setupToolbar() -- 工具栏
- setupStatusBar() -- 创建状态栏 QLabel
- connectSignals() -- 信号编排 (连接相关的信号将大幅简化)
- loadSettings() / saveSettings() -- 设置 (saveSettings 通过 m_connController->currentConnection() 查询)
- onDisplayModeChanged() / onTimestampToggled() / onClearTerminal() -- 终端 UI 控制
- onExportData() -- 数据导出
- onSearchRequested() / onSearchCleared() -- 搜索
- onThemeChanged() / onLanguageChanged() -- 主题/语言
- updateDataStatistics() -- 统计刷新
- onConnectionStateChangedUI() / onConnectionFailedUI() / onByteCountersUpdated() / onBreathingAnimationRequested() -- UI 更新槽
- closeEvent()

---

## 九、MainWindow connectSignals() 重构

### 9.1 连接相关的信号连接变更

**删除** (原来直接连接到 MainWindow slot):

```cpp
// 删除: 串口连接/断开 (改为委托 ConnectionController)
connect(m_serialConfig, &SerialConfigPanel::connectRequested,
        this, &MainWindow::onConnectSerial);                    // 删除
connect(m_serialConfig, &SerialConfigPanel::disconnectRequested,
        this, &MainWindow::onDisconnectSerial);                 // 删除

// 删除: SendController::dataSent 直接触发 updateStatusBar (改为触发 ConnectionController)
connect(m_sendController, &SendController::dataSent,
        this, [this](qint64) { updateStatusBar(); });           // 删除

// 删除: RecordingController::playbackData 中的 updateStatusBar 调用
connect(m_recordingController, &RecordingController::playbackData,
        this, [this](const QByteArray& data, qint64 direction) {
            if (direction == 0) m_terminalModel->appendReceived(data);
            else m_terminalModel->appendSent(data);
            updateStatusBar();                                   // 删除此行
        });

// 删除: 导航树网络节点直接调用 onConnectNetwork
if (text == tr("TCP客户端")) { onConnectNetwork(ConnectionType::TcpClient); return; }  // 删除
if (text == tr("TCP服务端")) { onConnectNetwork(ConnectionType::TcpServer); return; }  // 删除
if (text == tr("UDP")) { onConnectNetwork(ConnectionType::Udp); return; }              // 删除
```

**新增**:

```cpp
// 串口连接/断开 --> 委托 ConnectionController
connect(m_serialConfig, &SerialConfigPanel::connectRequested,
        this, [this]() {
    m_connController->connectSerial([this]() -> QVariantMap {
        QVariantMap params;
        params["portName"] = m_serialConfig->currentPortData();
        params["baudRate"] = m_serialConfig->currentBaudRate();
        params["dataBits"] = m_serialConfig->currentDataBitsIndex() + 5;
        params["parity"] = m_serialConfig->currentParityIndex();
        params["stopBits"] = m_serialConfig->currentStopBitsIndex();
        params["flowControl"] = m_serialConfig->currentFlowControlIndex();
        params["dtr"] = true;
        params["rts"] = true;
        return params;
    });
});
connect(m_serialConfig, &SerialConfigPanel::disconnectRequested,
        m_connController, &ConnectionController::disconnectSerial);

// 网络连接 --> 委托 ConnectionController
// (导航树网络节点点击时)

// ConnectionController 信号 --> MainWindow UI 更新
connect(m_connController, &ConnectionController::connectionStateChanged,
        this, &MainWindow::onConnectionStateChangedUI);
connect(m_connController, &ConnectionController::connectionFailed,
        this, &MainWindow::onConnectionFailedUI);
connect(m_connController, &ConnectionController::byteCountersUpdated,
        this, &MainWindow::onByteCountersUpdated);
connect(m_connController, &ConnectionController::statusMessage,
        this, [this](const QString& msg, int timeoutMs) {
    statusBar()->showMessage(msg, timeoutMs);
});
connect(m_connController, &ConnectionController::navigateToTerminalRequested,
        this, [this]() {
    m_navController->switchToPanel(m_terminal);
});
connect(m_connController, &ConnectionController::serialConfigConnectedChanged,
        m_serialConfig, &SerialConfigPanel::setConnected);
connect(m_connController, &ConnectionController::breathingAnimationRequested,
        this, &MainWindow::onBreathingAnimationRequested);

// SendController::dataSent --> 刷新字节计数
connect(m_sendController, &SendController::dataSent,
        m_connController, [this](qint64) {
    m_connController->updateByteCounters();
});

// RecordingController::playbackData --> 刷新字节计数
connect(m_recordingController, &RecordingController::playbackData,
        this, [this](const QByteArray& data, qint64 direction) {
    if (direction == 0) m_terminalModel->appendReceived(data);
    else m_terminalModel->appendSent(data);
    m_connController->updateByteCounters();
});
```

### 9.2 导航树网络节点点击重构

```cpp
// 导航树点击处理中, 网络节点改为委托:
if (text == tr("TCP客户端")) { m_connController->connectNetwork(ConnectionType::TcpClient); return; }
if (text == tr("TCP服务端")) { m_connController->connectNetwork(ConnectionType::TcpServer); return; }
if (text == tr("UDP")) { m_connController->connectNetwork(ConnectionType::Udp); return; }
```

---

## 十、逐行迁移计划

### 10.1 MainWindow.cpp 迁出明细

| MainWindow.cpp 行号 | 内容 | 迁移目标 | 动作 |
|---------------------|------|---------|------|
| 21 | `m_connManager(new ConnectionManager(this))` | ConnectionController 构造函数 | 删除, ConnectionController 内部持有 |
| 98 | `m_currentConn = nullptr` 声明 | ConnectionController 成员 | 删除 |
| 250-253 | 串口连接/断开信号连接 | MainWindow::connectSignals 重写 | 简化为委托调用 |
| 260-261 | SendController::dataSent --> updateStatusBar | 改连到 ConnectionController | 修改 |
| 281-288 | RecordingController::playbackData lambda | 移除 updateStatusBar 调用 | 修改 |
| 333-335 | 导航树网络节点 | 委托 ConnectionController | 修改 |
| 411-454 | `onConnectSerial()` | ConnectionController::connectSerial() | 移动 |
| 456-464 | `onDisconnectSerial()` | ConnectionController::disconnectSerial() | 移动 |
| 550-593 | `onConnectNetwork(ConnectionType)` | ConnectionController::connectNetwork() | 移动 |
| 595-632 | `onConnectionStateChanged(ConnectionState)` | ConnectionController::onConnectionStateChanged() | 移动 |
| 634-640 | `onDataReceived(const QByteArray&)` | ConnectionController::onDataReceived() | 移动 |
| 642-655 | `updateStatusBar()` | ConnectionController::updateByteCounters() | 移动 |
| 667 | closeEvent 中停止呼吸动画 | 保留 (UI 操作) | 保留 |
| 677-680 | closeEvent 中关闭所有连接 | ConnectionController::closeAllConnections() | 委托 |

### 10.2 MainWindow.cpp 替换为委托调用

**onClearTerminal() 中:**

```cpp
// 原: updateStatusBar();
// 替换为:
m_connController->updateByteCounters();
```

**closeEvent() 中:**

```cpp
// 原: auto connections = m_connManager->connections(); for (...) conn->close();
// 替换为:
m_connController->closeAllConnections();
```

**saveSettings() 中:**

```cpp
// 不需要从连接读取配置, 串口配置始终从 m_serialConfig 读取, 无变化
```

---

## 十一、预期行数分析

### 11.1 迁出行数统计

| 迁出内容 | 行数 |
|---------|------|
| onConnectSerial() | 44 |
| onDisconnectSerial() | 9 |
| onConnectNetwork() | 44 |
| onConnectionStateChanged() | 38 |
| onDataReceived() | 7 |
| updateStatusBar() | 14 |
| closeEvent 中连接关闭 (4 行) | 4 |
| connectSignals 中串口连接/断开信号 (4 行) | 4 |
| m_connManager/m_currentConn 初始化 (2 行) | 2 |
| **合计迁出** | **~166** |

### 11.2 新增行数 (委托调用替代)

| 新增内容 | 行数 |
|---------|------|
| m_connController 构造 (初始化列表) | 1 |
| m_connController->setSendController/OtaManager/RecordingController | 3 |
| connectSerial lambda (配置读取) | 12 |
| disconnectSerial 信号连接 | 2 |
| 7 个 ConnectionController 信号连接 | 14 |
| 3 个导航树网络节点委托调用 | 3 |
| SendController::dataSent --> updateByteCounters | 2 |
| RecordingController::playbackData 修改 | 4 |
| onClearTerminal 中委托调用 | 1 |
| closeEvent 中委托调用 | 1 |
| 4 个新增 UI 更新槽函数实现 | 30 |
| #include "core/ConnectionController.h" | 1 |
| **合计新增** | **~74** |

### 11.3 ConnectionController.cpp 预计行数

| 内容 | 行数 |
|------|------|
| 头文件 + include | 12 |
| 构造函数 | 8 |
| 析构函数 | 5 |
| setSendController/setOtaManager/setRecordingController | 9 |
| connectSerial() | 40 |
| disconnectSerial() | 12 |
| connectNetwork() | 35 |
| currentConnection() | 3 |
| closeAllConnections() | 8 |
| updateByteCounters() | 14 |
| onConnectionStateChanged() | 30 |
| onDataReceived() | 8 |
| onConnectionError() | 5 |
| connectConnectionSignals() | 10 |
| formatBytes() (静态) | 6 |
| **合计** | **~205** |

### 11.4 最终预估

```
MainWindow.cpp 当前行数:          682
减去迁出:                       -166
加上新增委托代码:                +74
-----------------------------------
预估 MainWindow.cpp 行数:        ~590 (首版)

进一步优化 (onConnectionStateChangedUI 等 UI 槽函数):
  - onConnectionStateChangedUI: ~20 行 (QLabel 文本/property/style 更新)
  - onConnectionFailedUI: ~3 行 (QMessageBox)
  - onByteCountersUpdated: ~4 行 (QLabel setText)
  - onBreathingAnimationRequested: ~6 行 (调用 NavigationController)
  合计 UI 槽: ~33 行

  这些是 MainWindow 必须保留的 UI 代码, 无法进一步迁出。

  但 onConnectionStateChangedUI 取代了原来的 onConnectionStateChanged (38行),
  因为业务逻辑 (RecordingController::setConnected, SendController::setConnection)
  已经移入 ConnectionController, 只剩纯 UI 操作。

预估最终行数:                     ~450 行 (满足 < 500 行约束)
```

### 11.5 MainWindow.cpp 最终职责清单

| 方法 | 行数 | 职责 |
|------|------|------|
| 构造/析构 | 45 | 创建所有组件, 构建布局 |
| setupUI() | 95 | 纯 UI 布局 |
| setupToolbar() | 55 | 工具栏 |
| setupStatusBar() | 12 | 状态栏 |
| connectSignals() | 80 | 信号编排 |
| loadSettings()/saveSettings() | 45 | 设置读写 |
| onDisplayModeChanged/onTimestampToggled/onClearTerminal | 15 | 终端 UI |
| onExportData() | 30 | 数据导出 |
| onSearchRequested/onSearchCleared | 8 | 搜索 |
| onThemeChanged/onLanguageChanged | 15 | 主题/语言 |
| onConnectionStateChangedUI | 20 | 连接状态 UI |
| onConnectionFailedUI | 3 | 连接失败弹窗 |
| onByteCountersUpdated | 4 | RX/TX 显示 |
| onBreathingAnimationRequested | 6 | 呼吸动画控制 |
| updateDataStatistics | 5 | 统计刷新 |
| closeEvent | 15 | 关闭清理 |
| **合计** | **~453** | |

---

## 十二、构造顺序与生命周期

### 12.1 MainWindow 构造函数变更

```cpp
MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , m_terminalModel(new TerminalModel(this))
    , m_sendHistory(new SendHistory(this))
    , m_dataExporter(new DataExporter(this))
    , m_dataLogger(new DataLogger(this))
    , m_recordingController(new RecordingController(m_dataLogger, this))
    , m_sendController(new SendController(m_terminalModel, m_dataLogger, m_sendHistory, this))
    , m_statsTimer(new QTimer(this))
    , m_frameParser(new FrameParser(this))
    , m_otaManager(new OtaManager(this))
    , m_navController(new NavigationController(this))
    , m_connController(new ConnectionController(
          new ConnectionManager(this),     // ConnectionManager 作为子对象
          m_terminalModel,
          m_frameParser,
          m_dataLogger,
          this))
{
    // 注入业务层协作对象 (构造后注入, 避免循环依赖)
    m_connController->setSendController(m_sendController);
    m_connController->setOtaManager(m_otaManager);
    m_connController->setRecordingController(m_recordingController);

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

注意: `m_connManager` 和 `m_currentConn` 不再出现在 MainWindow 中,
全部移入 ConnectionController。

### 12.2 对象所有权

```
MainWindow (QObject parent)
  |
  +-- TerminalModel*                    (QObject, this)
  +-- SendHistory*                      (QObject, this)
  +-- DataExporter*                     (QObject, this)
  +-- DataLogger*                       (QObject, this)
  +-- NavigationController*             (QObject, this)
  +-- RecordingController*              (QObject, this)
  +-- SendController*                   (QObject, this)
  +-- OtaManager*                       (QObject, this)
  +-- ConnectionController*             (QObject, this)   <-- 新增
  |     |
  |     +-- ConnectionManager*          (QObject, m_connController)
  |     |     |
  |     |     +-- IConnection*          (动态创建/销毁, 由 ConnectionManager 管理)
  |     |
  |     +-- m_currentConn               (引用, 由 ConnectionManager 拥有)
  |
  +-- QTreeView* m_navTree              (QObject, this)
  +-- QLabel* m_connStatusLbl           (QObject, this)   <-- 仅 MainWindow 操作
  +-- QLabel* m_rxBytesLbl              (QObject, this)   <-- 仅 MainWindow 操作
  +-- QLabel* m_txBytesLbl              (QObject, this)   <-- 仅 MainWindow 操作
  +-- ... (其他 UI 组件)
```

ConnectionController 是 MainWindow 的子对象, 生命周期由 Qt 父子树管理。
ConnectionManager 作为 ConnectionController 的子对象, 连接实例由 ConnectionManager 拥有。

### 12.3 构造顺序依赖链

```
TerminalModel ──> SendController ──> ConnectionController
DataLogger   ──/                   │
FrameParser  ─────────────────────>│
                                   ├──> setSendController()
                                   ├──> setOtaManager()
                                   └──> setRecordingController()
```

ConnectionController 构造时只需要数据层依赖 (TerminalModel, FrameParser, DataLogger)
和基础设施层依赖 (ConnectionManager)。业务层协作对象通过 setter 延迟注入,
打破构造顺序循环依赖。

---

## 十三、SerialConfigProvider 设计理由

`connectSerial()` 接受一个 `std::function<QVariantMap()>` 而非直接接受 `QVariantMap` 参数,
原因如下:

1. **解耦**: ConnectionController 不持有 SerialConfigPanel 引用 (表现层组件), 无法自行读取配置。
2. **延迟读取**: 配置在连接按钮被点击时才从 SerialConfigPanel 读取, 而非在构造时固定。
3. **灵活性**: 未来如果配置来源变化 (如从设置文件恢复上次配置), 只需更改 lambda,
   ConnectionController 无需任何修改。
4. **模式一致**: 类似于 SendController::createSendBar() 返回 QWidget 的模式,
   ConnectionController 不感知具体的 UI 控件, 只通过回调获取数据。

---

## 十四、onConnectionStateChangedUI 实现细节

这是 MainWindow 中最关键的 UI 更新槽函数, 替代原来 38 行的 `onConnectionStateChanged`:

```cpp
void MainWindow::onConnectionStateChangedUI(ConnectionState state, const QString& name)
{
    const char* stateStr = "";
    switch (state) {
    case ConnectionState::Connected:
        m_connStatusLbl->setText(tr("已连接: %1").arg(name));
        stateStr = "connected";
        // 业务层同步已在 ConnectionController 内完成, 此处仅 UI 操作
        break;
    case ConnectionState::Disconnected:
        m_connStatusLbl->setText(tr("未连接"));
        stateStr = "disconnected";
        break;
    case ConnectionState::Connecting:
        m_connStatusLbl->setText(tr("连接中..."));
        stateStr = "connecting";
        break;
    case ConnectionState::Error:
        m_connStatusLbl->setText(tr("连接错误"));
        stateStr = "error";
        break;
    }
    m_connStatusLbl->setProperty("state", stateStr);
    m_connStatusLbl->style()->unpolish(m_connStatusLbl);
    m_connStatusLbl->style()->polish(m_connStatusLbl);
}
```

从 38 行降至约 20 行, 因为:
- `m_serialConfig->setConnected()` -- 通过信号 `serialConfigConnectedChanged` 直接连接, 不在此处
- `m_recordingController->setConnected()` -- 由 ConnectionController 内部直接调用
- `m_navController->switchToPanel()` -- 通过信号 `navigateToTerminalRequested` 直接连接
- `m_navController->startBreathingAnimation/stopBreathingAnimation()` -- 通过信号 `breathingAnimationRequested` 触发 `onBreathingAnimationRequested` 槽

---

## 十五、CMakeLists.txt 变更

在 `src/core` 对应的源文件列表中新增:

```cmake
# 新增源文件
src/core/ConnectionController.h
src/core/ConnectionController.cpp
```

---

## 十六、公共组件清单更新

新增 ConnectionController 到 CLAUDE.md 4.4 公共组件清单:

| 组件 | 文件 | 用途 |
|------|------|------|
| `ConnectionController` | `core/ConnectionController.h/cpp` | 连接控制器 (连接创建/断开/状态同步/数据分发/字节统计) |

---

## 十七、验证清单

| 检查项 | 预期结果 |
|-------|---------|
| ConnectionController 编译通过 | 零错误 |
| MainWindow.cpp 行数 | ~450 (满足 < 500) |
| ConnectionController.cpp 行数 | ~205 |
| 串口连接: 点击连接按钮, 创建连接, 配置参数, 打开成功 | 行为不变 |
| 串口连接失败: QMessageBox 弹出 "Cannot open serial port" | 行为不变 |
| 串口断开: 点击断开, 关闭连接, 清理资源 | 行为不变 |
| TCP 客户端连接: 点击导航树, 创建连接, 默认参数 | 行为不变 |
| TCP 服务端连接: 点击导航树, 创建连接 | 行为不变 |
| UDP 连接: 点击导航树, 创建连接 | 行为不变 |
| 连接状态变化: QLabel 文本/颜色/呼吸动画正确 | 行为不变 |
| 接收数据: 正确分发到终端/帧解析/日志 | 行为不变 |
| RX/TX 字节统计: 状态栏实时更新 | 行为不变 |
| SendController 正确收到 setConnection 注入 | 行为不变 |
| OtaManager 正确收到 setConnection 注入 | 行为不变 |
| RecordingController 正确收到 setConnected 通知 | 行为不变 |
| SerialConfigPanel 正确收到 setConnected 通知 | 行为不变 |
| 连接成功后自动切换到终端面板 | 行为不变 |
| closeEvent 正确关闭所有连接 | 行为不变 |
| EmbedDebug.bat 启动正常 | 行为不变 |
| 分层合规: ConnectionController 不依赖任何表现层组件 | 零违规 |
