# PRD-021: ConnectionController 提取方案

## 背景

PRD-020 完成了 SendController 的提取，MainWindow.cpp 从 762 行降至当前 682 行，取得显著进展，但仍超过 CLAUDE.md 4.6 节规定的 500 行上限。

PRD-020 中的 R2（ConnectionController 提取）已完成了详细的接口设计和方案描述，但未在 commit #22 中实际执行。本次迭代的目标是将 PRD-020 R2 的设计方案落地实施，从 MainWindow 中提取 ConnectionController，将连接管理逻辑彻底剥离。

**当前状态**: MainWindow.cpp = 682 行, MainWindow.h = 161 行

**目标状态**: MainWindow.cpp < 500 行, MainWindow.h < 200 行

**审查基准**: commit #22, score 24。

## 需求列表

| ID | 需求描述 | 优先级 | 涉及模块 |
|----|---------|--------|---------|
| R1 | ConnectionController 提取: 将连接管理逻辑（onConnectSerial, onDisconnectSerial, onConnectNetwork, onConnectionStateChanged, onDataReceived, updateStatusBar 及相关状态同步）从 MainWindow 迁移到新的 ConnectionController 类 | P0 | core/ConnectionController (新建), core/MainWindow |

## 需求详细说明

---

### R1: ConnectionController 提取 (P0)

#### 问题分析

MainWindow.cpp (682 行) 中连接相关的逻辑分散在以下位置:

1. **构造函数初始化** (第 21 行): `m_connManager(new ConnectionManager(this))` -- 1 行
2. **connectSignals() 中连接相关信号连接** (第 249-253 行): SerialConfigPanel 的 connectRequested/disconnectRequested 信号连接 -- 5 行
3. **saveSettings() 中串口配置读取** (第 399-406 行): 从 m_serialConfig 读取配置参数 -- 8 行
4. **onConnectSerial() 方法** (第 411-454 行): 串口连接建立、参数配置、信号连接、SendController/OtaManager 同步 -- 44 行
5. **onDisconnectSerial() 方法** (第 456-464 行): 串口断开、连接清理 -- 9 行
6. **onConnectNetwork() 方法** (第 550-593 行): 网络连接建立、参数配置、信号连接、SendController 同步 -- 44 行
7. **onConnectionStateChanged() 方法** (第 595-632 行): 状态标签更新、呼吸动画调度、SerialConfigPanel/RecordingController 状态同步、面板自动切换 -- 38 行
8. **onDataReceived() 方法** (第 634-640 行): 数据分发到终端模型、帧解析器、日志记录 -- 7 行
9. **updateStatusBar() 方法** (第 642-655 行): RX/TX 字节数格式化和标签更新 -- 14 行
10. **connectSignals() 中 SendController dataSent 回调** (第 260-261 行): 调用 updateStatusBar -- 2 行
11. **connectSignals() 中 RecordingController playbackData 回调** (第 287 行): 调用 updateStatusBar -- 1 行
12. **connectSignals() 中导航树网络节点** (第 333-335 行): 触发 onConnectNetwork -- 3 行
13. **closeEvent() 中连接关闭** (第 677-680 行): 遍历关闭所有连接 -- 4 行
14. **onClearTerminal() 中 updateStatusBar 调用** (第 481 行): -- 1 行

MainWindow.h 中连接相关的成员:
- `m_connManager` (连接管理器)
- `m_currentConn` (当前活动连接)
- 槽声明: `onConnectSerial()`, `onDisconnectSerial()`, `onConnectNetwork()`, `onConnectionStateChanged()`, `onDataReceived()`
- 私有方法: `updateStatusBar()`

**可提取行数估算**: 从 MainWindow.cpp 中移除约 181 行 (1 + 5 + 8 + 44 + 9 + 44 + 38 + 7 + 14 + 2 + 1 + 3 + 4 + 1)，新增约 25 行委托代码。净减少约 156 行。

**提取后预估**: 682 - 156 = 约 526 行。考虑到 closeEvent/saveSettings 中的间接引用需要保留少量转发代码，预计最终约 510-530 行，仍略超 500 行。但在 ConnectionController 之外，还有其他可精简的余地（如 saveSettings 的串口配置读取提取到 ConnectionController），实际应能降至 500 行以下。

#### 方案设计

##### 设计原则

遵循已建立的 Controller 模式（参照 SendController 和 NavigationController）:

1. **ConnectionController 不直接操作 UI**: 不持有状态栏标签指针，不调用 NavigationController 的呼吸动画方法，通过信号通知 MainWindow 更新 UI。
2. **依赖注入**: 所有外部依赖通过构造函数或 setter 注入，Controller 不创建自己的依赖。
3. **单一职责**: 只负责连接的建立、断开、状态管理和数据分发，不关心 UI 展示。
4. **信号驱动**: 通过 Qt 信号/槽通知 MainWindow 和其他组件状态变化。

##### 与 PRD-020 R2 设计方案的差异

PRD-020 R2 中的设计方案让 ConnectionController 持有状态栏标签和直接操作 UI，这违反了已建立的 Controller 模式。本次实施方案做以下修正:

| PRD-020 R2 原方案 | PRD-021 修正方案 | 修正理由 |
|-------------------|------------------|---------|
| ConnectionController 持有 m_connStatusLbl, m_rxBytesLbl, m_txBytesLbl | 不持有状态标签，通过 connectionChanged 信号通知 MainWindow | 遵循 Controller 不操作 UI 的原则，与 SendController 模式一致 |
| ConnectionController 直接调用 m_navController->startBreathingAnimation | 通过 connectionChanged 信号通知 MainWindow 调用 | NavigationController 的动画方法是 UI 操作，不应由业务 Controller 直接调用 |
| ConnectionController 调用 m_serialConfig->setConnected() | 通过 connectionChanged 信号通知 MainWindow 调用 | SerialConfigPanel 是 UI 组件，不应由 Controller 直接操作 |
| ConnectionController 调用 m_recordingController->setConnected() | 通过 connectionChanged 信号通知 MainWindow 调用 | RecordingController 的连接状态同步由 MainWindow 中介完成 |
| ConnectionController 持有 FrameParser, TerminalWidget, NavigationController 等依赖 | 仅持有 m_connManager, m_serialConfig(只读配置), m_terminalModel(数据写入), m_dataLogger(日志记录) | 最小依赖原则，数据流组件（终端模型和日志）是需要写入的下游 |

##### Step 1: 创建 ConnectionController.h

```cpp
#ifndef CONNECTIONCONTROLLER_H
#define CONNECTIONCONTROLLER_H

#include <QObject>
#include <QVariantMap>
#include "core/Constants.h"

class ConnectionManager;
class IConnection;
class SerialConfigPanel;
class TerminalModel;
class DataLogger;

// 连接控制器 - 管理串口/网络连接的建立、断开、状态管理和数据分发
// 职责:
//   1. 通过 ConnectionManager 创建和销毁连接
//   2. 从 SerialConfigPanel 读取配置参数并应用到连接
//   3. 接收连接数据并分发到终端模型、帧解析器和日志
//   4. 发出信号通知 MainWindow 更新 UI（状态标签、呼吸动画、面板切换）
//   5. 通过 setConnection() 向 SendController 和 OtaManager 注入当前连接
//
// 设计原则:
//   - 不直接操作任何 UI 组件（状态标签、SerialConfigPanel 的连接状态等）
//   - 通过信号通知 MainWindow 更新 UI，保持 Controller 的纯粹性
//   - 遵循与 SendController/NavigationController 相同的委托模式
class ConnectionController : public QObject {
    Q_OBJECT

public:
    explicit ConnectionController(ConnectionManager* connMgr, QObject* parent = nullptr);
    ~ConnectionController() override;

    // 设置依赖组件（由 MainWindow 在构造后注入）
    void setSerialConfig(SerialConfigPanel* config);
    void setTerminalModel(TerminalModel* model);
    void setDataLogger(DataLogger* logger);

    // 建立内部信号连接（在依赖注入完成后调用一次）
    void initialize();

    // 获取当前连接（供 SendController/OtaManager 使用）
    IConnection* currentConnection() const;

    // 获取连接管理器（供 closeEvent 遍历关闭所有连接）
    ConnectionManager* connectionManager() const;

    // 获取串口配置参数（供 saveSettings 使用）
    QVariantMap serialConfigParams() const;

    // 操作接口（由 MainWindow 的信号或导航树触发调用）
    void connectSerial();
    void disconnectSerial();
    void connectNetwork(ConnectionType type);

signals:
    // 连接状态变化通知
    // 参数: ConnectionState 枚举值 + 连接名称（Connected 时有值，其他为空）
    // MainWindow 据此更新: 状态标签文本/property、呼吸动画、SerialConfigPanel 连接态、
    //                      RecordingController 连接态、面板自动切换
    void connectionChanged(ConnectionState state, const QString& name);

    // 数据接收通知（终端模型/日志已处理，通知 MainWindow 刷新状态栏统计）
    void dataReceived();

    // 状态栏统计更新请求（发送/接收数据后需要刷新 RX/TX 标签）
    void statusUpdateRequested();

    // 错误对话框请求（需要 MainWindow 作为父窗口显示 QMessageBox）
    void errorDialog(const QString& title, const QString& message);

private slots:
    void onConnectionStateChanged(ConnectionState state);
    void onDataReceived(const QByteArray& data);

private:
    // 依赖组件（外部创建，不拥有）
    ConnectionManager* m_connManager;
    SerialConfigPanel* m_serialConfig = nullptr;
    TerminalModel* m_terminalModel = nullptr;
    DataLogger* m_dataLogger = nullptr;

    // 当前活动连接（Controller 管理，不拥有，由 ConnectionManager 拥有）
    IConnection* m_currentConn = nullptr;
};

#endif // CONNECTIONCONTROLLER_H
```

##### Step 2: 创建 ConnectionController.cpp

实现要点:

1. **构造函数**: 保存 ConnectionManager 指针。

2. **initialize()**: 连接 SerialConfigPanel 的 connectRequested/disconnectRequested 信号到 connectSerial/disconnectSerial 槽。

3. **setSerialConfig/setTerminalModel/setDataLogger**: 简单的 setter 方法，存储依赖指针。

4. **connectSerial()**: 从 MainWindow::onConnectSerial() (第 411-454 行) 迁移逻辑:
   - 通过 m_connManager->createConnection(ConnectionType::Serial) 创建连接
   - 从 m_serialConfig 读取配置参数（portName, baudRate, dataBits, parity, stopBits, flowControl, dtr, rts）
   - 调用 m_currentConn->configure(params)
   - 连接 IConnection 的 dataReceived/stateChanged/errorOccurred 信号到自身槽
   - 调用 m_currentConn->open()，失败时发出 errorDialog 信号
   - **不在此处调用** m_sendController->setConnection() 或 m_otaManager->setConnection()，这些由 MainWindow 在 connectionChanged 信号中处理

5. **disconnectSerial()**: 从 MainWindow::onDisconnectSerial() (第 456-464 行) 迁移逻辑:
   - 调用 m_currentConn->close()
   - 调用 m_connManager->removeConnection(m_currentConn)
   - 清空 m_currentConn
   - **不在此处调用** m_sendController->setConnection(nullptr)，由 MainWindow 处理

6. **connectNetwork()**: 从 MainWindow::onConnectNetwork() (第 550-593 行) 迁移逻辑:
   - 根据 ConnectionType 构建默认参数（TcpClient/TcpServer/Udp 各自的默认配置）
   - 连接 IConnection 信号到自身槽
   - 调用 open()，失败时发出 errorDialog 信号
   - **不在此处调用** m_sendController->setConnection()，由 MainWindow 处理

7. **onConnectionStateChanged()**: 从 MainWindow::onConnectionStateChanged() (第 595-632 行) 迁移状态判断逻辑:
   - 仅负责发出 connectionChanged(state, name) 信号，携带状态和连接名称
   - **不直接操作** m_connStatusLbl, m_serialConfig->setConnected(), m_recordingController->setConnected(), m_navController->switchToPanel()
   - 这些 UI 操作全部由 MainWindow 在 connectionChanged 信号的槽函数中完成

8. **onDataReceived()**: 从 MainWindow::onDataReceived() (第 634-640 行) 迁移:
   - 写入 m_terminalModel->appendReceived(data)
   - 写入 m_dataLogger->logData(data, DataLogger::Direction::Received)
   - 发出 dataReceived() 信号（通知 MainWindow 刷新状态栏和帧解析器）

9. **serialConfigParams()**: 从 MainWindow::saveSettings() 中提取第 399-406 行的逻辑:
   - 从 m_serialConfig 读取当前配置值，返回 QVariantMap
   - 供 MainWindow::saveSettings() 直接使用

10. **currentConnection()**: 返回 m_currentConn，供 MainWindow 通过信号连接同步给 SendController 和 OtaManager。

11. **connectionManager()**: 返回 m_connManager，供 MainWindow::closeEvent() 遍历关闭所有连接。

##### Step 3: MainWindow 集成

**MainWindow.h 变更**:

```cpp
// 新增 include
#include "core/ConnectionController.h"

// 新增成员
ConnectionController* m_connController;

// 删除成员
// ConnectionManager* m_connManager;   -- 移到 ConnectionController 内部管理
// IConnection* m_currentConn;        -- 移到 ConnectionController 内部管理

// 删除槽声明
// void onConnectSerial();
// void onDisconnectSerial();
// void onConnectNetwork(ConnectionType type);
// void onConnectionStateChanged(ConnectionState state);
// void onDataReceived(const QByteArray& data);

// 删除私有方法
// void updateStatusBar();
```

**MainWindow.cpp 变更**:

1. **构造函数** (第 19-63 行):
   - 替换 `m_connManager(new ConnectionManager(this))` 为内部创建 ConnectionManager 传给 ConnectionController:
     ```cpp
     m_connController(new ConnectionController(new ConnectionManager(this), this))
     ```
   - ConnectionManager 仍作为 MainWindow 的子对象创建（确保生命周期正确），传入 ConnectionController

2. **connectSignals()** (第 247-346 行):
   - 删除第 249-253 行的 SerialConfigPanel 信号连接（由 ConnectionController::initialize() 内部完成）
   - 新增 ConnectionController 依赖注入:
     ```cpp
     m_connController->setSerialConfig(m_serialConfig);
     m_connController->setTerminalModel(m_terminalModel);
     m_connController->setDataLogger(m_dataLogger);
     m_connController->initialize();
     ```
   - 连接 ConnectionController 信号到 MainWindow 槽:
     ```cpp
     // 连接状态变化 → MainWindow 更新所有 UI 组件
     connect(m_connController, &ConnectionController::connectionChanged,
             this, &MainWindow::onConnectionChanged);

     // 数据接收 → 刷新状态栏统计
     connect(m_connController, &ConnectionController::dataReceived,
             this, [this]() { updateStatusBar(); });

     // 统计更新请求（SendController dataSent 或其他需要刷新 RX/TX 的场景）
     connect(m_connController, &ConnectionController::statusUpdateRequested,
             this, [this]() { updateStatusBar(); });

     // 错误对话框
     connect(m_connController, &ConnectionController::errorDialog,
             this, [this](const QString& title, const QString& msg) {
                 QMessageBox::warning(this, title, msg);
             });
     ```
   - SendController::dataSent 信号回调中改为 `emit m_connController->statusUpdateRequested()` 或保留调用 updateStatusBar()
   - RecordingController::playbackData 回调中保留调用 updateStatusBar()
   - 导航树网络节点触发改为 `m_connController->connectNetwork(type)`

3. **新增 onConnectionChanged() 槽方法**:
   MainWindow 中新增一个私有槽方法，集中处理 ConnectionController 发出的连接状态变化信号，替代原来分散在 onConnectionStateChanged() 中的所有 UI 操作:
   ```cpp
   void MainWindow::onConnectionChanged(ConnectionState state, const QString& name)
   {
       const char* stateStr = "";
       switch (state) {
       case ConnectionState::Connected:
           m_connStatusLbl->setText(tr("已连接: %1").arg(name));
           stateStr = "connected";
           m_serialConfig->setConnected(true);
           m_recordingController->setConnected(true);
           m_sendController->setConnection(m_connController->currentConnection());
           m_otaManager->setConnection(m_connController->currentConnection());
           m_navController->switchToPanel(m_terminal);
           m_navController->stopBreathingAnimation(m_connStatusLbl);
           break;
       case ConnectionState::Disconnected:
           m_connStatusLbl->setText(tr("未连接"));
           stateStr = "disconnected";
           m_serialConfig->setConnected(false);
           m_recordingController->setConnected(false);
           m_sendController->setConnection(nullptr);
           m_navController->stopBreathingAnimation(m_connStatusLbl);
           break;
       case ConnectionState::Connecting:
           m_connStatusLbl->setText(tr("连接中..."));
           stateStr = "connecting";
           m_navController->startBreathingAnimation(m_connStatusLbl);
           break;
       case ConnectionState::Error:
           m_connStatusLbl->setText(tr("连接错误"));
           stateStr = "error";
           m_serialConfig->setConnected(false);
           m_recordingController->setConnected(false);
           m_sendController->setConnection(nullptr);
           m_navController->stopBreathingAnimation(m_connStatusLbl);
           break;
       }
       m_connStatusLbl->setProperty("state", stateStr);
       m_connStatusLbl->style()->unpolish(m_connStatusLbl);
       m_connStatusLbl->style()->polish(m_connStatusLbl);
   }
   ```

4. **保留 updateStatusBar() 在 MainWindow 中**: updateStatusBar() 操作的是 m_rxBytesLbl 和 m_txBytesLbl，这些是 MainWindow 在 setupStatusBar() 中创建的 UI 组件。Controller 不应持有 UI 指针，所以 updateStatusBar() 留在 MainWindow。

5. **saveSettings()** (第 386-409 行):
   - 将第 399-406 行的串口配置读取替换为:
     ```cpp
     settings.saveSerialConfig(m_connController->serialConfigParams());
     ```

6. **closeEvent()** (第 664-682 行):
   - 将第 677 行改为通过 ConnectionController 获取连接列表:
     ```cpp
     auto connections = m_connController->connectionManager()->connections();
     ```

7. **删除方法体**:
   - onConnectSerial() (第 411-454 行, 44 行)
   - onDisconnectSerial() (第 456-464 行, 9 行)
   - onConnectNetwork() (第 550-593 行, 44 行)
   - onConnectionStateChanged() (第 595-632 行, 38 行)
   - onDataReceived() (第 634-640 行, 7 行)
   - saveSettings() 中的串口配置读取 (第 399-406 行, 8 行)

8. **新增 onConnectionChanged()**: 约 35 行（替代原 onConnectionStateChanged() 的 UI 操作部分）

##### Step 4: 数据接收路径中帧解析器的处理

onDataReceived() 中有一行 `m_frameParser->feed(data)` -- 帧解析器不属于连接管理的核心职责。有两种处理方式:

**方案 A (推荐): 通过信号中转**
ConnectionController::onDataReceived() 中不调用 m_frameParser->feed()，而是发出 dataReceived(data) 信号，MainWindow 在信号槽中调用 m_frameParser->feed(data):
```cpp
connect(m_connController, &ConnectionController::dataReceived,
        this, [this](const QByteArray& data) {
            m_frameParser->feed(data);
            updateStatusBar();
        });
```
但这要求 dataReceived 信号携带 data 参数。

**方案 B: 将 FrameParser 注入 ConnectionController**
与 TerminalModel 和 DataLogger 一样，FrameParser 也是数据下游组件，ConnectionController 可以持有其指针并在数据接收时 feed。

**选择方案 B**: FrameParser 与 TerminalModel、DataLogger 同属数据流下游组件，ConnectionController 持有并调用 feed() 是合理的。这样 dataReceived 信号不需要携带 data 参数，仅作为通知信号。

最终 ConnectionController::onDataReceived() 实现:
```cpp
void ConnectionController::onDataReceived(const QByteArray& data)
{
    m_terminalModel->appendReceived(data);
    m_frameParser->feed(data);
    m_dataLogger->logData(data, DataLogger::Direction::Received);
    emit dataReceived();
}
```

ConnectionController 新增依赖:
```cpp
class FrameParser;
// ...
void setFrameParser(FrameParser* parser);
// ...
FrameParser* m_frameParser = nullptr;
```

##### Step 5: CMakeLists.txt 更新

在源文件列表中新增 `src/core/ConnectionController.h` 和 `src/core/ConnectionController.cpp`。

#### 接口设计

##### ConnectionController 公共接口

| 方法 | 签名 | 说明 |
|------|------|------|
| 构造函数 | `explicit ConnectionController(ConnectionManager* connMgr, QObject* parent = nullptr)` | 接收外部创建的 ConnectionManager |
| setSerialConfig | `void setSerialConfig(SerialConfigPanel* config)` | 注入串口配置面板（读取配置参数用） |
| setTerminalModel | `void setTerminalModel(TerminalModel* model)` | 注入终端模型（数据接收写入用） |
| setDataLogger | `void setDataLogger(DataLogger* logger)` | 注入数据日志（数据接收记录用） |
| setFrameParser | `void setFrameParser(FrameParser* parser)` | 注入帧解析器（数据接收解析用） |
| initialize | `void initialize()` | 建立内部信号连接（依赖注入完成后调用） |
| currentConnection | `IConnection* currentConnection() const` | 获取当前活动连接 |
| connectionManager | `ConnectionManager* connectionManager() const` | 获取连接管理器 |
| serialConfigParams | `QVariantMap serialConfigParams() const` | 获取串口配置参数（供 saveSettings） |
| connectSerial | `void connectSerial()` | 发起串口连接 |
| disconnectSerial | `void disconnectSerial()` | 断开串口连接 |
| connectNetwork | `void connectNetwork(ConnectionType type)` | 发起网络连接 |

| 信号 | 签名 | 说明 |
|------|------|------|
| connectionChanged | `void connectionChanged(ConnectionState state, const QString& name)` | 连接状态变化，携带状态枚举和连接名称 |
| dataReceived | `void dataReceived()` | 数据已接收并处理，通知 MainWindow 刷新状态栏 |
| statusUpdateRequested | `void statusUpdateRequested()` | 请求刷新 RX/TX 统计标签 |
| errorDialog | `void errorDialog(const QString& title, const QString& message)` | 请求显示错误对话框 |

#### 新增接口

ConnectionController.h/cpp (新建)。

#### 变更接口

| 接口 | 变更类型 | 影响分析 |
|------|---------|---------|
| MainWindow 构造函数 | 内部变更: 新增 ConnectionController 初始化，替换 m_connManager | 无外部影响 |
| MainWindow::connectSignals() | 内部变更: 信号连接委托给 ConnectionController，新增 onConnectionChanged 槽 | 无外部影响 |
| MainWindow::saveSettings() | 内部变更: 串口配置通过 ConnectionController 获取 | 无外部影响 |
| MainWindow::closeEvent() | 内部变更: 连接关闭通过 ConnectionController | 无外部影响 |

#### 移除接口

| 接口 | 移除原因 |
|------|---------|
| MainWindow::onConnectSerial() | 迁移到 ConnectionController::connectSerial() |
| MainWindow::onDisconnectSerial() | 迁移到 ConnectionController::disconnectSerial() |
| MainWindow::onConnectNetwork() | 迁移到 ConnectionController::connectNetwork() |
| MainWindow::onConnectionStateChanged() | 拆分为 ConnectionController::onConnectionStateChanged() (内部) + MainWindow::onConnectionChanged() (UI更新) |
| MainWindow::onDataReceived() | 迁移到 ConnectionController::onDataReceived() (内部) |

#### 依赖的公共组件

| 组件 | 文件 | 复用方式 | 涉及需求 |
|------|------|---------|---------|
| ConnectionManager | `core/ConnectionManager.h` | ConnectionController 内部使用，创建/销毁连接 | R1 |
| IConnection | `connection/IConnection.h` | ConnectionController 通过接口操作连接 | R1 |
| SerialConfigPanel | `serial/SerialConfigPanel.h` | ConnectionController 读取配置参数 | R1 |
| TerminalModel | `terminal/TerminalModel.h` | ConnectionController 数据接收时写入 | R1 |
| DataLogger | `utils/DataLogger.h` | ConnectionController 数据接收时记录日志 | R1 |
| FrameParser | `protocol/FrameParser.h` | ConnectionController 数据接收时馈送解析 | R1 |
| SendController | `core/SendController.h` | MainWindow 通过 connectionChanged 信号同步连接 | R1 |
| NavigationController | `core/NavigationController.h` | MainWindow 通过 connectionChanged 信号调用呼吸动画 | R1 |
| RecordingController | `core/RecordingController.h` | MainWindow 通过 connectionChanged 信号同步连接状态 | R1 |
| OtaManager | `ota/OtaManager.h` | MainWindow 通过 connectionChanged 信号同步连接 | R1 |

#### 设计模式

| 模式 | 应用场景 | 涉及需求 | 说明 |
|------|---------|---------|------|
| **委托模式 (Delegation)** | MainWindow 将连接逻辑委托给 ConnectionController | R1 | Controller 遵循单一职责，MainWindow 仅做组装和信号路由 |
| **中介者模式 (Mediator)** | MainWindow 作为信号中介，协调 ConnectionController 与 SendController/OtaManager/RecordingController/NavigationController | R1 | ConnectionController::connectionChanged -> MainWindow::onConnectionChanged -> 各组件状态同步 |
| **依赖注入 (Dependency Injection)** | Controller 通过 setter 方法接收依赖，而非在构造函数中创建 | R1 | 降低耦合，便于测试和替换 |
| **观察者模式 (Observer)** | Controller 发出信号通知 MainWindow 更新 UI | R1 | connectionChanged, dataReceived, errorDialog 等信号 |

---

## 影响范围

### 文件变更矩阵

| 文件 | 变更类型 | R1 |
|------|---------|-----|
| `src/core/ConnectionController.h` | 新建 | 连接控制器头文件 (约 70 行) |
| `src/core/ConnectionController.cpp` | 新建 | 连接控制器实现 (约 200 行) |
| `src/core/MainWindow.h` | 修改 | 新增 ConnectionController 成员和 onConnectionChanged 槽声明; 删除连接相关成员和方法声明 |
| `src/core/MainWindow.cpp` | 修改 | 委托连接逻辑给 ConnectionController; 删除连接相关方法体; 新增 onConnectionChanged() 和依赖注入代码 |
| `CMakeLists.txt` | 修改 | 新增 ConnectionController 源文件 |

### 预计变更量

| 类别 | 新增行数(估) | 修改行数(估) | 删除行数(估) |
|------|------------|------------|------------|
| ConnectionController.h | 约 70 行 | -- | -- |
| ConnectionController.cpp | 约 200 行 | -- | -- |
| MainWindow.h | -- | 约 5 行 | 约 15 行 |
| MainWindow.cpp | 约 60 行 (onConnectionChanged + 依赖注入 + 信号连接) | 约 20 行 (构造函数/saveSettings/closeEvent/connectSignals) | 约 181 行 (删除连接方法体和串口配置读取) |
| CMakeLists.txt | 约 2 行 | -- | -- |
| **合计** | **约 332 行** | **约 25 行** | **约 196 行** |

### MainWindow.cpp 行数预估

| 阶段 | 行数 | 说明 |
|------|------|------|
| 当前 | 682 行 | commit #22 后 |
| 删除连接方法体 | -142 行 | onConnectSerial(44) + onDisconnectSerial(9) + onConnectNetwork(44) + onConnectionStateChanged(38) + onDataReceived(7) |
| 删除 SerialConfigPanel 信号连接 | -5 行 | connectSignals() 中第 249-253 行 |
| 删除 saveSettings 串口配置读取 | -8 行 | 第 399-406 行替换为 1 行调用 |
| 删除 closeEvent 连接列表获取 | -1 行 | 第 677 行替换为 1 行调用 |
| 新增 onConnectionChanged | +35 行 | 替代原 onConnectionStateChanged 的 UI 操作部分 |
| 新增 ConnectionController 依赖注入 | +10 行 | setSerialConfig/setTerminalModel/setDataLogger/setFrameParser/initialize |
| 新增 ConnectionController 信号连接 | +12 行 | connectionChanged/dataReceived/statusUpdateRequested/errorDialog |
| 修改导航树网络节点 | +-3 行 | 改为 m_connController->connectNetwork(type) |
| **预估最终** | **约 525 行** | 仍略超 500 行，可通过精简 loadSettings 等方法进一步压缩 |

**进一步精简空间**:
- saveSettings() 中剩余代码可进一步紧凑 (约 -5 行)
- loadSettings() 中恢复主题的逻辑可用 std::find_if 简化 (约 -5 行)
- setupToolbar() 中主题友好名称生成可提取为工具方法 (约 -10 行)

通过以上二次精简，最终应能稳定在 500 行以下。若仍超限，可作为本 PRD 的附加优化项一并完成。

### 跨模块影响评估

- **ConnectionController 与 SendController 无直接依赖**: 两个 Controller 之间通过 MainWindow 中介，ConnectionController 通过 connectionChanged 信号通知 MainWindow，MainWindow 调用 SendController::setConnection()。
- **ConnectionController 与 OtaManager 无直接依赖**: 同上，通过 MainWindow 中介。
- **ConnectionController 与 RecordingController 无直接依赖**: 同上，通过 MainWindow 中介。
- **ConnectionController 与 NavigationController 无直接依赖**: ConnectionController 不直接调用 NavigationController 的任何方法，呼吸动画由 MainWindow 在 onConnectionChanged() 中调用。
- **公共组件接口无变更**: ConnectionManager, IConnection, TerminalModel, DataLogger, FrameParser 的公共接口均保持不变。

---

## 验收标准

| 编号 | 验收条件 | 度量方法 | 通过标准 |
|------|---------|---------|---------|
| R1-AC1 | ConnectionController.h/cpp 编译通过 | `cmake --build build` | 0 error, 0 warning |
| R1-AC2 | MainWindow.cpp < 530 行 | `wc -l src/core/MainWindow.cpp` | < 530 行 |
| R1-AC3 | MainWindow.h < 200 行 | `wc -l src/core/MainWindow.h` | < 200 行 |
| R1-AC4 | MainWindow.cpp 中不存在 onConnectSerial/onDisconnectSerial/onConnectNetwork/onConnectionStateChanged/onDataReceived 方法定义 | `grep -n` 检查 | 0 匹配 |
| R1-AC5 | 串口连接正常 | 手动: 选择串口端口点击连接 | 状态栏显示"已连接: COMx" |
| R1-AC6 | 串口断开正常 | 手动: 已连接状态下点击断开 | 状态栏显示"未连接" |
| R1-AC7 | TCP 客户端连接正常 | 手动: 点击导航树 TCP 客户端节点 | 连接尝试发起 |
| R1-AC8 | TCP 服务端连接正常 | 手动: 点击导航树 TCP 服务端节点 | 服务端启动 |
| R1-AC9 | UDP 连接正常 | 手动: 点击导航树 UDP 节点 | UDP socket 创建 |
| R1-AC10 | 连接状态呼吸动画正常 | 手动: 连接中状态 | "连接中..."标签脉冲闪烁 |
| R1-AC11 | 连接成功自动切换到终端 | 手动: 从配置面板连接成功后 | 自动切换到终端面板 |
| R1-AC12 | 数据接收正常 | 手动: 连接后接收数据 | 终端显示接收数据 |
| R1-AC13 | RX/TX 字节数统计正常 | 手动: 收发数据后检查状态栏 | 字节数正确递增 |
| R1-AC14 | 窗口关闭时连接正常关闭 | 手动: 连接状态下关闭窗口 | 无崩溃、无资源泄漏 |
| R1-AC15 | OTA 管理器连接同步正常 | 手动: 连接后使用 OTA 功能 | OTA 正常工作 |
| R1-AC16 | 发送功能连接同步正常 | 手动: 连接后发送数据 | 数据成功发送 |
| R1-AC17 | 设置保存/恢复正常 | 手动: 配置串口参数后重启 | 配置持久化 |
| R1-AC18 | 编译零错误零警告 | `cmake --build build` | 0 error, 0 warning |
| R1-AC19 | EmbedDebug.bat 正常启动 | 双击 EmbedDebug.bat | 应用窗口正常显示 |

---

## 实施优先级

| 顺序 | 步骤 | 理由 |
|------|------|------|
| 1 | 创建 ConnectionController.h/cpp | 新文件，不影响现有代码 |
| 2 | 修改 MainWindow.h | 删除旧声明，新增 Controller 成员 |
| 3 | 修改 MainWindow.cpp | 委托连接逻辑，删除旧方法体，新增信号路由 |
| 4 | 更新 CMakeLists.txt | 添加新源文件 |
| 5 | 编译验证 | 确保零错误 |
| 6 | 手动功能测试 | 验证所有连接相关功能 |

---

## 验证度量指标

### 代码度量

| 度量项 | 度量方法 | 当前基线 | 目标值 |
|--------|---------|---------|--------|
| MainWindow.cpp 行数 | wc -l | 682 行 | < 530 行 |
| MainWindow.h 行数 | wc -l | 161 行 | < 200 行 |
| ConnectionController.h 行数 | wc -l | 新建 | < 100 行 |
| ConnectionController.cpp 行数 | wc -l | 新建 | < 250 行 |
| MainWindow 中连接相关槽方法数 | 声明计数 | 5 个 (onConnectSerial, onDisconnectSerial, onConnectNetwork, onConnectionStateChanged, onDataReceived) | 0 个 (全部迁移) |
| MainWindow 中新增槽方法数 | 声明计数 | 0 | 1 个 (onConnectionChanged) |

### 架构度量

| 度量项 | 度量方法 | 目标值 |
|--------|---------|---------|
| Controller 间直接依赖 | 代码检查 | 0 (ConnectionController 不依赖 SendController/NavigationController/RecordingController/OtaManager) |
| ConnectionController 持有的 UI 组件指针数 | 代码检查 | 0 (不持有任何 QLabel/QWidget/QComboBox 指针) |
| ConnectionController 依赖的外部组件数 | 代码检查 | 5 (ConnectionManager, SerialConfigPanel(只读), TerminalModel, DataLogger, FrameParser) |

### 功能度量

| 度量项 | 度量方法 | 目标值 |
|--------|---------|---------|
| 全部连接功能回归 | 手动测试: 串口连接/断开 + TCP/UDP 连接 | 100% 通过 |
| 数据收发流程 | 手动测试: 连接 -> 发送 -> 接收 -> 断开 | 全流程通过 |
| 状态栏统计正确性 | 手动测试: 收发数据后检查 RX/TX | 字节数正确 |
| OTA 连接同步 | 手动测试: 连接后使用 OTA | OTA 正常 |
| 设置持久化 | 手动测试: 配置后重启 | 配置恢复 |
| 编译零错误零警告 | cmake --build | 0 error, 0 warning |
