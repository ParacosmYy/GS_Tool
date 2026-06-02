# PRD-020: MainWindow Final Split -- SendController + ConnectionController + sendAndRecord Bug Fix

## 背景

PRD-019 完成了第一轮 MainWindow 拆分，将 NavigationController 和 RecordingController 的冗余代码从 MainWindow 中移除。MainWindow.cpp 从 1092 行降至当前 762 行，取得显著进展，但仍超过 CLAUDE.md 4.6 节规定的 500 行上限。

本次迭代是 MainWindow 拆分的最终阶段，目标是通过提取两个新 Controller 将 MainWindow.cpp 降至 500 行以下，同时修复代码审查中发现的一个 sendAndRecord 关键缺陷。

**当前状态**: MainWindow.cpp = 762 行, MainWindow.h = 182 行

**目标状态**: MainWindow.cpp < 500 行, MainWindow.h < 200 行

**审查基准**: PRD-019 交付后，当前评分 23/1000，commit #22。

## 需求列表

| ID | 需求描述 | 优先级 | 涉及模块 |
|----|---------|--------|---------|
| R1 | SendController 提取: 将发送相关逻辑（onSendData, onQuickCommand, sendAndRecord 及相关 UI 组件管理）迁移到新的 SendController 类 | P0 | core/SendController (新建), core/MainWindow |
| R2 | ConnectionController 提取: 将连接相关逻辑（onConnectSerial, onDisconnectSerial, onConnectNetwork, onConnectionStateChanged, onDataReceived 及连接状态 UI 更新）迁移到新的 ConnectionController 类 | P0 | core/ConnectionController (新建), core/MainWindow |
| R3 | 修复 sendAndRecord 静默失败: 当 write() 返回 0 时，当前代码无任何用户反馈，用户以为发送成功但数据实际未发出。需通过状态栏消息通知用户发送失败 | P0 | core/SendController (迁移后修复) |

## 需求详细说明

---

### R1: SendController 提取 (P0)

#### 问题分析

MainWindow.cpp 中发送相关的逻辑分散在多个位置:

1. **构造函数初始化**: m_timedSender, m_sendHistory (第 23-24 行)
2. **setupUI() 中发送区域创建**: m_sendModeCombo, m_sendInput, m_sendBtn, m_sendCompleter, m_sendCompleterModel (第 158-187 行, 约 30 行)
3. **connectSignals() 中发送相关连接**: 发送按钮/回车/快捷指令/定时发送器/发送历史 (第 283-288, 331-338 行, 约 14 行)
4. **业务方法**: sendAndRecord() (第 497-511 行, 15 行), onSendData() (第 513-543 行, 31 行), onQuickCommand() (第 545-548 行, 4 行)

MainWindow.h 中发送相关的成员:
- m_sendHistory, m_timedSender (业务对象)
- m_sendInput, m_sendBtn, m_sendModeCombo, m_sendCompleter, m_sendCompleterModel (UI 组件)
- sendAndRecord(), onSendData(), onQuickCommand() (方法声明)

**可提取行数估算**: 约 94 行 (30 + 14 + 15 + 31 + 4) 从 MainWindow.cpp 移除，但 MainWindow 需要新增少量转发代码。净减少约 70-80 行。

#### 方案设计

**Step 1: 创建 SendController.h**

```cpp
#ifndef SENDCONTROLLER_H
#define SENDCONTROLLER_H

#include <QObject>
#include <QStringListModel>
#include <QCompleter>

class QLineEdit;
class QPushButton;
class QComboBox;
class IConnection;
class SendHistory;
class TimedSender;
class DataLogger;
class TerminalModel;

// 发送控制器 - 管理数据发送、发送历史、定时发送
// 从 MainWindow 中提取的发送相关逻辑，遵循单一职责原则
class SendController : public QObject {
    Q_OBJECT

public:
    explicit SendController(SendHistory* history, TimedSender* timedSender,
                            DataLogger* logger, QObject* parent = nullptr);
    ~SendController() override;

    // 创建发送区域 UI 组件（由 MainWindow 在 setupUI 时调用）
    // 返回包含 sendModeCombo + sendInput + sendBtn 的 QFrame
    QWidget* createSendBar(QWidget* parent = nullptr);

    // 设置终端模型（用于显示已发送数据）
    void setTerminalModel(TerminalModel* model);

    // 设置当前连接（发送数据时使用）
    void setCurrentConnection(IConnection* conn);

    // 建立内部信号连接（在 UI 组件和依赖都就绪后调用）
    void initialize();

    // 触发快捷指令发送
    void sendQuickCommand(const QByteArray& data);

    // 获取发送输入框（MainWindow 用于恢复焦点等操作）
    QLineEdit* sendInput() const;

signals:
    // 状态更新通知（MainWindow 用于刷新状态栏）
    void statusChanged();

    // 状态栏消息（发送失败时通知用户）
    void statusMessage(const QString& msg, int timeoutMs = 0);

private slots:
    void onSendData();
    void onTimedSend(const QByteArray& data);
    void onHistoryChanged();

private:
    // 统一发送方法: 写入连接 + 记录终端 + 日志 + 更新状态栏
    // 返回实际写入字节数（0 = 失败/未连接，>0 = 成功）
    qint64 sendAndRecord(const QByteArray& data, bool isHex = false);

    // 依赖组件（外部创建，不拥有）
    SendHistory* m_sendHistory;
    TimedSender* m_timedSender;
    DataLogger* m_dataLogger;
    TerminalModel* m_terminalModel = nullptr;
    IConnection* m_currentConn = nullptr;

    // UI 组件（由 createSendBar 创建，SendController 拥有）
    QLineEdit* m_sendInput = nullptr;
    QPushButton* m_sendBtn = nullptr;
    QComboBox* m_sendModeCombo = nullptr;
    QCompleter* m_sendCompleter = nullptr;
    QStringListModel* m_sendCompleterModel = nullptr;
};

#endif // SENDCONTROLLER_H
```

**Step 2: 创建 SendController.cpp**

实现要点:

1. `createSendBar()`: 将 MainWindow::setupUI() 中第 158-187 行的发送区域 UI 创建代码迁移过来。创建 QFrame 容器，内部包含 m_sendModeCombo、m_sendInput、m_sendBtn，布局与当前一致。返回 QFrame 供 MainWindow 添加到 serialLayout。

2. `initialize()`: 连接内部信号:
   - m_sendBtn clicked -> onSendData
   - m_sendInput returnPressed -> onSendData
   - m_timedSender sendData -> onTimedSend
   - m_sendHistory historyChanged -> onHistoryChanged

3. `onSendData()`: 从 MainWindow 迁移第 513-543 行逻辑。从 m_sendInput 读取文本，根据 m_sendModeCombo 判断文本/HEX 模式，调用 sendAndRecord，成功后添加历史并清空输入框。

4. `onQuickCommand()` -> `sendQuickCommand()`: 直接调用 sendAndRecord(data)。

5. `onTimedSend()`: 调用 sendAndRecord(data)。

6. `sendAndRecord()`: 从 MainWindow 迁移第 497-511 行逻辑。检查连接状态，调用 write()，成功则记录到终端和日志。

7. `onHistoryChanged()`: 更新 m_sendCompleterModel 的字符串列表。

**Step 3: MainWindow 集成**

MainWindow.h 变更:
- 新增 `#include "core/SendController.h"`
- 新增成员: `SendController* m_sendController`
- 删除成员: `m_sendInput`, `m_sendBtn`, `m_sendModeCombo`, `m_sendCompleter`, `m_sendCompleterModel`
- 删除槽声明: `onSendData()`, `onQuickCommand()`
- 删除私有方法: `sendAndRecord()`
- 保留 `m_sendHistory`, `m_timedSender`（构造时传给 SendController，仍由 MainWindow 拥有）

MainWindow.cpp 变更:
- 构造函数: 新增 `m_sendController(new SendController(m_sendHistory, m_timedSender, m_dataLogger, this))`
- setupUI(): 将发送区域 UI 创建替换为 `serialLayout->addWidget(m_sendController->createSendBar())`
- connectSignals():
  - 删除发送按钮/回车/快捷指令/定时发送器/发送历史的连接（约 14 行）
  - 新增: `m_sendController->setTerminalModel(m_terminalModel)`
  - 新增: `m_sendController->initialize()`
  - 新增: 连接 `m_sendController::statusChanged` -> `MainWindow::updateStatusBar`
  - 新增: 连接 `m_sendController::statusMessage` -> status bar
  - 快捷指令: `m_quickCmdBar::commandTriggered` -> `m_sendController->sendQuickCommand`
- 删除方法体: `onSendData()`, `onQuickCommand()`, `sendAndRecord()`

**Step 4: CMakeLists.txt 更新**

在源文件列表中新增 `src/core/SendController.h` 和 `src/core/SendController.cpp`。

#### 影响范围

| 文件 | 变更内容 |
|------|---------|
| `src/core/SendController.h` | 新建: 发送控制器头文件 |
| `src/core/SendController.cpp` | 新建: 发送控制器实现 |
| `src/core/MainWindow.h` | 新增 SendController 成员; 删除发送相关 UI 成员和方法声明 |
| `src/core/MainWindow.cpp` | 委托发送逻辑给 SendController; 删除 onSendData/onQuickCommand/sendAndRecord 方法体 |
| `CMakeLists.txt` | 新增 SendController 源文件 |

#### 验收标准

| 编号 | 验收条件 | 度量方法 | 通过标准 |
|------|---------|---------|---------|
| R1-AC1 | SendController.h/cpp 编译通过 | cmake --build | 0 error |
| R1-AC2 | 文本模式发送正常 | 手动: 连接后输入文本点击发送 | 终端显示已发送数据 |
| R1-AC3 | HEX 模式发送正常 | 手动: 切换 HEX 模式输入 "AA 55" 点击发送 | 终端显示 HEX 数据 |
| R1-AC4 | HEX 输入无效时显示错误态 | 手动: HEX 模式输入 "GG" 点击发送 | 输入框变红 |
| R1-AC5 | 回车键发送正常 | 手动: 输入文本按回车 | 数据发出 |
| R1-AC6 | 快捷指令发送正常 | 手动: 点击 AT/Reset/Status 按钮 | 数据发出 |
| R1-AC7 | 定时发送正常 | 手动: 配置定时发送并启动 | 数据按时发送 |
| R1-AC8 | 发送历史自动补全正常 | 手动: 发送后清空输入框重新输入部分字符 | 下拉列表出现历史匹配 |
| R1-AC9 | 发送历史记录正确 | 手动: 发送多条命令后检查历史 | 记录正确、去重正常 |

---

### R2: ConnectionController 提取 (P0)

#### 问题分析

MainWindow.cpp 中连接相关的逻辑分散在多个位置:

1. **构造函数初始化**: m_connManager (第 21 行)
2. **setupUI() 中配置面板创建**: m_serialConfig (第 103-105 行, 3 行)
3. **setupStatusBar()**: m_connStatusLbl, m_rxBytesLbl, m_txBytesLbl (第 263-272 行, 约 10 行)
4. **connectSignals() 中连接相关连接**: 串口连接/断开、导航树网络节点、播放数据回写后更新状态栏 (第 277-280, 366-368, 309-311 行, 约 14 行)
5. **saveSettings()**: 串口配置保存 (第 433-438 行, 6 行)
6. **业务方法**: onConnectSerial() (第 444-486 行, 43 行), onDisconnectSerial() (第 488-495 行, 8 行), onConnectNetwork() (第 634-674 行, 41 行), onConnectionStateChanged() (第 676-710 行, 35 行), onDataReceived() (第 712-718 行, 7 行)
7. **辅助方法**: updateStatusBar() (第 720-733 行, 14 行)

MainWindow.h 中连接相关的成员:
- m_connManager, m_currentConn (连接管理)
- m_serialConfig (配置面板引用)
- m_connStatusLbl, m_rxBytesLbl, m_txBytesLbl (状态栏 UI)
- onConnectSerial(), onDisconnectSerial(), onConnectNetwork(), onConnectionStateChanged(), onDataReceived() (方法声明)

**可提取行数估算**: 约 181 行 (3 + 10 + 14 + 6 + 43 + 8 + 41 + 35 + 7 + 14) 从 MainWindow.cpp 移除。但 MainWindow 需要新增少量转发和接口代码。净减少约 150-160 行。

#### 方案设计

**Step 1: 创建 ConnectionController.h**

```cpp
#ifndef CONNECTIONCONTROLLER_H
#define CONNECTIONCONTROLLER_H

#include <QObject>
#include "core/Constants.h"

class QLabel;
class SerialConfigPanel;
class ConnectionManager;
class IConnection;
class DataLogger;
class TerminalModel;
class FrameParser;
class OtaManager;
class NavigationController;
class TerminalWidget;

// 连接控制器 - 管理串口/网络连接的建立、断开、状态显示
// 从 MainWindow 中提取的连接相关逻辑，遵循单一职责原则
class ConnectionController : public QObject {
    Q_OBJECT

public:
    explicit ConnectionController(ConnectionManager* connMgr, QObject* parent = nullptr);
    ~ConnectionController() override;

    // 设置依赖组件（由 MainWindow 在构造后注入）
    void setSerialConfig(SerialConfigPanel* config);
    void setDataLogger(DataLogger* logger);
    void setTerminalModel(TerminalModel* model);
    void setFrameParser(FrameParser* parser);
    void setOtaManager(OtaManager* otaMgr);
    void setNavigationController(NavigationController* nav);
    void setTerminal(TerminalWidget* terminal);

    // 设置状态栏标签（由 MainWindow 在 setupStatusBar 后注入）
    void setStatusLabels(QLabel* connStatus, QLabel* rxBytes, QLabel* txBytes);

    // 建立内部信号连接
    void initialize();

    // 获取当前连接（供 SendController 使用）
    IConnection* currentConnection() const;

    // 获取连接管理器
    ConnectionManager* connectionManager() const;

    // 获取串口配置参数（用于 saveSettings）
    QVariantMap serialConfigParams() const;

    // 操作接口
    void connectSerial();
    void disconnectSerial();
    void connectNetwork(ConnectionType type);

signals:
    // 连接状态变化通知（MainWindow 用于更新面板可见性等）
    void connectionStateChanged(ConnectionState state);

    // 数据接收通知（MainWindow 用于更新统计等）
    void dataReceived();

    // 状态更新通知（MainWindow 用于刷新统计等 UI）
    void statusUpdated();

    // 状态栏消息
    void statusMessage(const QString& msg, int timeoutMs = 0);

    // 错误对话框（需要 MainWindow 作为父窗口）
    void errorDialog(const QString& title, const QString& message);

    // 连接成功后自动切换到终端面板的请求
    void switchToTerminalRequested();

private slots:
    void onConnectionStateChanged(ConnectionState state);
    void onDataReceived(const QByteArray& data);

private:
    void updateStatusBar();

    // 依赖组件（外部创建，不拥有）
    ConnectionManager* m_connManager;
    SerialConfigPanel* m_serialConfig = nullptr;
    DataLogger* m_dataLogger = nullptr;
    TerminalModel* m_terminalModel = nullptr;
    FrameParser* m_frameParser = nullptr;
    OtaManager* m_otaManager = nullptr;
    NavigationController* m_navController = nullptr;
    TerminalWidget* m_terminal = nullptr;

    // 状态栏标签（外部创建，不拥有）
    QLabel* m_connStatusLbl = nullptr;
    QLabel* m_rxBytesLbl = nullptr;
    QLabel* m_txBytesLbl = nullptr;

    // 当前活动连接
    IConnection* m_currentConn = nullptr;
};

#endif // CONNECTIONCONTROLLER_H
```

**Step 2: 创建 ConnectionController.cpp**

实现要点:

1. `initialize()`: 连接 SerialConfigPanel 的 connectRequested/disconnectRequested 信号到 connectSerial/disconnectSerial 槽。

2. `connectSerial()`: 从 MainWindow 迁移第 444-486 行逻辑。通过 m_connManager 创建连接，从 m_serialConfig 读取配置参数，连接 IConnection 信号到自身槽，调用 open()，成功后设置到 m_otaManager。

3. `disconnectSerial()`: 从 MainWindow 迁移第 488-495 行逻辑。关闭并移除连接。

4. `connectNetwork()`: 从 MainWindow 迁移第 634-674 行逻辑。根据 ConnectionType 构建默认参数，连接信号，打开连接。

5. `onConnectionStateChanged()`: 从 MainWindow 迁移第 676-710 行逻辑。更新状态标签文本和 property，调用 m_navController 呼吸动画，更新 m_serialConfig 连接状态。发出 connectionStateChanged 和 switchToTerminalRequested 信号。

6. `onDataReceived()`: 从 MainWindow 迁移第 712-718 行逻辑。写入终端模型、帧解析器、数据日志，更新状态栏。发出 dataReceived 和 statusUpdated 信号。

7. `updateStatusBar()`: 从 MainWindow 迁移第 720-733 行逻辑。格式化 RX/TX 字节数并更新标签。

8. `serialConfigParams()`: 从 MainWindow::saveSettings() 提取串口配置读取逻辑（第 433-438 行），返回 QVariantMap 供 MainWindow 保存。

**Step 3: MainWindow 集成**

MainWindow.h 变更:
- 新增 `#include "core/ConnectionController.h"`
- 新增成员: `ConnectionController* m_connController`
- 删除成员: `m_connManager`, `m_currentConn`（移到 ConnectionController 内部管理）
- 删除槽声明: `onConnectSerial()`, `onDisconnectSerial()`, `onConnectNetwork()`, `onConnectionStateChanged()`, `onDataReceived()`
- 删除私有方法: `updateStatusBar()`
- 保留 m_serialConfig（setupUI 中创建，通过 setSerialConfig 注入 ConnectionController）
- 保留 m_connStatusLbl, m_rxBytesLbl, m_txBytesLbl（setupStatusBar 中创建，通过 setStatusLabels 注入 ConnectionController）

MainWindow.cpp 变更:
- 构造函数:
  - 将 `m_connManager(new ConnectionManager(this))` 替换为 `m_connController(new ConnectionController(new ConnectionManager(this), this))`
  - 注意: ConnectionManager 仍作为 MainWindow 的子对象创建，传给 ConnectionController
- setupUI(): m_serialConfig 创建不变，但不再在 connectSignals 中直接连接其信号
- setupStatusBar(): 状态栏标签创建不变，但之后调用 `m_connController->setStatusLabels(...)`
- connectSignals():
  - 删除串口连接/断开的信号连接（第 277-280 行）
  - 新增: `m_connController->setSerialConfig(m_serialConfig)` 等依赖注入
  - 新增: `m_connController->initialize()`
  - 导航树中的网络节点触发改为 `m_connController->connectNetwork(type)`
  - 连接 ConnectionController 的信号到 MainWindow 的处理
  - RecordingController 的 playbackData 回调中 updateStatusBar 改为 `m_connController->currentConnection()` 检查（如需要）
- saveSettings(): `m_serialConfig->currentPortData()` 等改为通过 `m_connController->serialConfigParams()` 获取
- closeEvent(): `m_connManager->connections()` 改为 `m_connController->connectionManager()->connections()`
- 删除方法体: onConnectSerial(), onDisconnectSerial(), onConnectNetwork(), onConnectionStateChanged(), onDataReceived(), updateStatusBar()

**Step 4: SendController 与 ConnectionController 协作**

SendController 需要当前连接来发送数据。两种方案:

方案 A (推荐): MainWindow 在 ConnectionController::connectionStateChanged 信号中，将当前连接同步给 SendController:
```cpp
connect(m_connController, &ConnectionController::connectionStateChanged,
        this, [this](ConnectionState state) {
            m_sendController->setCurrentConnection(
                state == ConnectionState::Connected
                    ? m_connController->currentConnection()
                    : nullptr);
        });
```

方案 B: SendController 持有 ConnectionController 指针，每次发送时查询 currentConnection()。

方案 A 更解耦，SendController 不依赖 ConnectionController，推荐使用。

**Step 5: CMakeLists.txt 更新**

在源文件列表中新增 `src/core/ConnectionController.h` 和 `src/core/ConnectionController.cpp`。

#### 影响范围

| 文件 | 变更内容 |
|------|---------|
| `src/core/ConnectionController.h` | 新建: 连接控制器头文件 |
| `src/core/ConnectionController.cpp` | 新建: 连接控制器实现 |
| `src/core/MainWindow.h` | 新增 ConnectionController 成员; 删除连接相关成员和方法声明 |
| `src/core/MainWindow.cpp` | 委托连接逻辑给 ConnectionController; 删除连接相关方法体 |
| `src/core/SendController.h` | setCurrentConnection() 方法（R1 中已定义） |
| `CMakeLists.txt` | 新增 ConnectionController 源文件 |

#### 验收标准

| 编号 | 验收条件 | 度量方法 | 通过标准 |
|------|---------|---------|---------|
| R2-AC1 | ConnectionController.h/cpp 编译通过 | cmake --build | 0 error |
| R2-AC2 | 串口连接正常 | 手动: 选择串口端口点击连接 | 状态栏显示"已连接: COMx" |
| R2-AC3 | 串口断开正常 | 手动: 已连接状态下点击断开 | 状态栏显示"未连接" |
| R2-AC4 | TCP 客户端连接正常 | 手动: 点击导航树 TCP 客户端节点 | 连接尝试发起 |
| R2-AC5 | TCP 服务端连接正常 | 手动: 点击导航树 TCP 服务端节点 | 服务端启动 |
| R2-AC6 | UDP 连接正常 | 手动: 点击导航树 UDP 节点 | UDP socket 创建 |
| R2-AC7 | 连接状态呼吸动画正常 | 手动: 连接中状态 | "连接中..."标签脉冲闪烁 |
| R2-AC8 | 连接成功自动切换到终端 | 手动: 从配置面板连接成功后 | 自动切换到终端面板 |
| R2-AC9 | 数据接收正常 | 手动: 连接后接收数据 | 终端显示接收数据 |
| R2-AC10 | RX/TX 字节数统计正常 | 手动: 收发数据后检查状态栏 | 字节数正确递增 |
| R2-AC11 | 窗口关闭时连接正常关闭 | 手动: 连接状态下关闭窗口 | 无崩溃、无资源泄漏 |
| R2-AC12 | OTA 管理器连接同步正常 | 手动: 连接后使用 OTA 功能 | OTA 正常工作 |

---

### R3: 修复 sendAndRecord 静默失败 (P0)

#### 问题分析

代码审查发现 `sendAndRecord()` 方法中存在一个关键缺陷:

```cpp
// 当前代码 (MainWindow.cpp 第 497-511 行)
bool MainWindow::sendAndRecord(const QByteArray& data, bool isHex)
{
    Q_UNUSED(isHex);
    if (!m_currentConn || m_currentConn->state() != ConnectionState::Connected) {
        return false;
    }
    qint64 written = m_currentConn->write(data);
    if (written > 0) {
        m_terminalModel->appendSent(data);
        m_dataLogger->logData(data, DataLogger::Direction::Sent);
        updateStatusBar();
        return true;
    }
    return false;   // <--- write() 返回 0 或 -1 时，无任何用户反馈
}
```

**问题**: 当 `write()` 返回 0 (底层缓冲区满/暂时无法写入) 或 -1 (写入错误) 时，`sendAndRecord()` 静默返回 false。调用者 `onSendData()` 在 sendAndRecord 返回 false 时不做任何处理，用户看到的现象是：点击发送后输入框没有清空（因为 clear 在 if 块内），也没有任何错误提示。用户不知道数据是否发出。

**影响场景**:
1. 串口硬件缓冲区满时连续快速发送
2. 网络连接不稳定导致 write 返回 0
3. 连接刚建立但底层尚未就绪
4. 部分写入场景（write 返回值小于 data.size()）

#### 方案设计

在 SendController 中修复此问题（迁移后修复，因此修改位置在 SendController::sendAndRecord）:

```cpp
qint64 SendController::sendAndRecord(const QByteArray& data, bool isHex)
{
    Q_UNUSED(isHex);
    if (!m_currentConn || m_currentConn->state() != ConnectionState::Connected) {
        emit statusMessage(tr("发送失败: 未连接"), 3000);
        return 0;
    }

    qint64 written = m_currentConn->write(data);
    if (written > 0) {
        m_terminalModel->appendSent(data);
        m_dataLogger->logData(data, DataLogger::Direction::Sent);
        emit statusChanged();
        return written;
    }

    // write() 返回 0 或 -1: 提供明确的用户反馈
    if (written == 0) {
        emit statusMessage(tr("发送失败: 缓冲区已满，请稍后重试"), 3000);
    } else {
        emit statusMessage(tr("发送失败: 写入错误"), 3000);
    }
    return written;
}
```

同时修改 `onSendData()` 对返回值的处理:

```cpp
void SendController::onSendData()
{
    if (!m_currentConn || m_currentConn->state() != ConnectionState::Connected) {
        emit statusMessage(tr("发送失败: 未连接"), 3000);
        return;
    }

    QString text = m_sendInput->text();
    if (text.isEmpty()) return;

    bool isHex = (m_sendModeCombo->currentIndex() == 1);
    QByteArray data;
    if (isHex) {
        data = HexConverter::fromHexString(text);
        if (data.isEmpty()) {
            m_sendInput->setProperty("hasError", true);
            m_sendInput->style()->unpolish(m_sendInput);
            m_sendInput->style()->polish(m_sendInput);
            return;
        }
    } else {
        data = text.toUtf8();
    }

    qint64 written = sendAndRecord(data);
    if (written > 0) {
        m_sendHistory->addEntry(text, isHex);
        m_sendInput->clear();
        m_sendInput->setProperty("hasError", false);
        m_sendInput->style()->unpolish(m_sendInput);
        m_sendInput->style()->polish(m_sendInput);
    }
    // written <= 0 时 sendAndRecord 已通过 statusMessage 通知用户
    // 不清空输入框，让用户可以重试
}
```

**设计决策**:
1. 返回类型从 `bool` 改为 `qint64`，提供更精确的写入字节数信息
2. 未连接时也通过 statusMessage 通知，而非静默 return
3. 发送失败时不清空输入框，保留内容供用户重试
4. 0 和 -1 分开提示，帮助用户判断原因
5. 使用 statusMessage 信号而非直接操作 statusBar，保持 SendController 不依赖 MainWindow 的 UI 组件

#### 影响范围

| 文件 | 变更内容 |
|------|---------|
| `src/core/SendController.cpp` | 修复 sendAndRecord 返回值处理，新增失败通知 |

与 R1 在同一文件中完成，无额外文件变更。

#### 验收标准

| 编号 | 验收条件 | 度量方法 | 通过标准 |
|------|---------|---------|---------|
| R3-AC1 | 未连接时发送有提示 | 手动: 未连接状态点击发送 | 状态栏显示"发送失败: 未连接" |
| R3-AC2 | write() 返回 0 时有提示 | 模拟: 连接但 write 返回 0 | 状态栏显示"发送失败: 缓冲区已满" |
| R3-AC3 | write() 返回 -1 时有提示 | 模拟: 连接但 write 返回 -1 | 状态栏显示"发送失败: 写入错误" |
| R3-AC4 | 发送失败时输入框内容保留 | 手动: 触发发送失败场景 | 输入框不清空，用户可重试 |
| R3-AC5 | 正常发送不受影响 | 手动: 正常连接下发送数据 | 数据发送成功，输入框清空 |

---

## 接口设计

### 新增接口

#### SendController

| 方法 | 签名 | 说明 |
|------|------|------|
| createSendBar | `QWidget* createSendBar(QWidget* parent = nullptr)` | 创建发送区域 UI 组件 |
| setTerminalModel | `void setTerminalModel(TerminalModel* model)` | 注入终端模型依赖 |
| setCurrentConnection | `void setCurrentConnection(IConnection* conn)` | 设置当前连接 |
| initialize | `void initialize()` | 建立内部信号连接 |
| sendQuickCommand | `void sendQuickCommand(const QByteArray& data)` | 触发快捷指令发送 |
| sendInput | `QLineEdit* sendInput() const` | 获取发送输入框 |

| 信号 | 签名 | 说明 |
|------|------|------|
| statusChanged | `void statusChanged()` | 发送后通知刷新状态栏 |
| statusMessage | `void statusMessage(const QString& msg, int timeoutMs)` | 状态栏消息通知 |

#### ConnectionController

| 方法 | 签名 | 说明 |
|------|------|------|
| setSerialConfig | `void setSerialConfig(SerialConfigPanel* config)` | 注入串口配置面板 |
| setDataLogger | `void setDataLogger(DataLogger* logger)` | 注入数据日志 |
| setTerminalModel | `void setTerminalModel(TerminalModel* model)` | 注入终端模型 |
| setFrameParser | `void setFrameParser(FrameParser* parser)` | 注入帧解析器 |
| setOtaManager | `void setOtaManager(OtaManager* mgr)` | 注入 OTA 管理器 |
| setNavigationController | `void setNavigationController(NavigationController* nav)` | 注入导航控制器 |
| setTerminal | `void setTerminal(TerminalWidget* terminal)` | 注入终端控件 |
| setStatusLabels | `void setStatusLabels(QLabel* conn, QLabel* rx, QLabel* tx)` | 注入状态栏标签 |
| initialize | `void initialize()` | 建立内部信号连接 |
| currentConnection | `IConnection* currentConnection() const` | 获取当前连接 |
| connectionManager | `ConnectionManager* connectionManager() const` | 获取连接管理器 |
| serialConfigParams | `QVariantMap serialConfigParams() const` | 获取串口配置参数 |
| connectSerial | `void connectSerial()` | 发起串口连接 |
| disconnectSerial | `void disconnectSerial()` | 断开串口连接 |
| connectNetwork | `void connectNetwork(ConnectionType type)` | 发起网络连接 |

| 信号 | 签名 | 说明 |
|------|------|------|
| connectionStateChanged | `void connectionStateChanged(ConnectionState state)` | 连接状态变化 |
| dataReceived | `void dataReceived()` | 数据接收通知 |
| statusUpdated | `void statusUpdated()` | 状态更新通知 |
| statusMessage | `void statusMessage(const QString& msg, int timeoutMs)` | 状态栏消息 |
| errorDialog | `void errorDialog(const QString& title, const QString& message)` | 错误对话框请求 |
| switchToTerminalRequested | `void switchToTerminalRequested()` | 请求切换到终端面板 |

### 变更接口

| 接口 | 变更类型 | 影响分析 |
|------|---------|---------|
| MainWindow 构造函数 | 内部变更: 新增 SendController/ConnectionController 初始化 | 无外部影响 |
| MainWindow::setupUI() | 内部变更: 发送区域委托给 SendController | 无外部影响 |
| MainWindow::setupStatusBar() | 内部变更: 状态标签注入到 ConnectionController | 无外部影响 |
| MainWindow::connectSignals() | 内部变更: 信号连接委托给 Controller | 无外部影响 |
| MainWindow::saveSettings() | 内部变更: 串口配置通过 ConnectionController 获取 | 无外部影响 |
| MainWindow::closeEvent() | 内部变更: 连接关闭通过 ConnectionController | 无外部影响 |

### 移除接口

| 接口 | 移除原因 |
|------|---------|
| MainWindow::onSendData() | 迁移到 SendController::onSendData() |
| MainWindow::onQuickCommand() | 迁移到 SendController::sendQuickCommand() |
| MainWindow::sendAndRecord() | 迁移到 SendController::sendAndRecord() |
| MainWindow::onConnectSerial() | 迁移到 ConnectionController::connectSerial() |
| MainWindow::onDisconnectSerial() | 迁移到 ConnectionController::disconnectSerial() |
| MainWindow::onConnectNetwork() | 迁移到 ConnectionController::connectNetwork() |
| MainWindow::onConnectionStateChanged() | 迁移到 ConnectionController |
| MainWindow::onDataReceived() | 迁移到 ConnectionController |
| MainWindow::updateStatusBar() | 迁移到 ConnectionController |

---

## 依赖的公共组件

| 组件 | 文件 | 复用方式 | 涉及需求 |
|------|------|---------|---------|
| IConnection | `connection/IConnection.h` | SendController/ConnectionController 通过接口操作连接 | R1, R2 |
| ConnectionManager | `core/ConnectionManager.h` | ConnectionController 内部使用 | R2 |
| SendHistory | `serial/SendHistory.h` | SendController 内部使用 | R1 |
| TimedSender | `serial/TimedSender.h` | SendController 内部使用 | R1 |
| DataLogger | `utils/DataLogger.h` | SendController/ConnectionController 内部使用 | R1, R2 |
| TerminalModel | `terminal/TerminalModel.h` | SendController/ConnectionController 内部使用 | R1, R2 |
| FrameParser | `protocol/FrameParser.h` | ConnectionController 内部使用 | R2 |
| OtaManager | `ota/OtaManager.h` | ConnectionController 内部使用 | R2 |
| NavigationController | `core/NavigationController.h` | ConnectionController 内部使用（呼吸动画、面板切换） | R2 |
| SerialConfigPanel | `serial/SerialConfigPanel.h` | ConnectionController 读取配置参数 | R2 |
| HexConverter | `utils/HexConverter.h` | SendController HEX 模式转换 | R1 |
| NavigationController | `core/NavigationController.h` | NavPanelMapping 结构体 | R2 |
| ThemeManager | `core/ThemeManager.h` | MainWindow 主题管理（不变） | -- |
| SettingsManager | `utils/SettingsManager.h` | MainWindow 设置读写（不变） | -- |

---

## 设计模式

| 模式 | 应用场景 | 涉及需求 | 说明 |
|------|---------|---------|------|
| **委托模式 (Delegation)** | MainWindow 将发送逻辑委托给 SendController，连接逻辑委托给 ConnectionController | R1, R2 | 每个 Controller 遵循单一职责，MainWindow 仅做组装和信号路由 |
| **中介者模式 (Mediator)** | MainWindow 作为信号中介，协调 SendController 和 ConnectionController | R1, R2 | ConnectionController::connectionStateChanged -> MainWindow -> SendController::setCurrentConnection |
| **依赖注入 (Dependency Injection)** | Controller 通过 setter 方法接收依赖，而非在构造函数中创建 | R1, R2 | 降低耦合，便于测试和替换 |
| **观察者模式 (Observer)** | Controller 发出信号通知 MainWindow 更新 UI | R1, R2 | statusChanged, statusMessage, connectionStateChanged 等信号 |

---

## 影响范围

### 文件变更矩阵

| 文件 | R1 | R2 | R3 | 新建/修改 |
|------|----|----|-----|---------|
| `src/core/SendController.h` | 新建 | -- | 包含 | 新建 |
| `src/core/SendController.cpp` | 新建 | -- | 包含 | 新建 |
| `src/core/ConnectionController.h` | -- | 新建 | -- | 新建 |
| `src/core/ConnectionController.cpp` | -- | 新建 | -- | 新建 |
| `src/core/MainWindow.h` | 精简 | 精简 | -- | 修改 |
| `src/core/MainWindow.cpp` | 精简 | 精简 | -- | 修改 |
| `CMakeLists.txt` | 新增源文件 | 新增源文件 | -- | 修改 |

### 预计变更量

| 需求 | 新增行数(估) | 修改行数(估) | 删除行数(估) |
|------|------------|------------|------------|
| R1 | SendController.h 约 60 行, SendController.cpp 约 150 行; MainWindow 新增约 15 行 | MainWindow 约 20 行 | MainWindow 约 94 行 |
| R2 | ConnectionController.h 约 70 行, ConnectionController.cpp 约 200 行; MainWindow 新增约 20 行 | MainWindow 约 25 行 | MainWindow 约 181 行 |
| R3 | 约 10 行 (错误处理分支) | 约 15 行 (sendAndRecord 和 onSendData) | 约 5 行 |
| **合计** | **约 525 行** (含两个新 Controller) | **约 60 行** | **约 280 行** |

### MainWindow.cpp 行数预估

| 阶段 | 行数 | 说明 |
|------|------|------|
| 当前 | 762 行 | PRD-019 后 |
| R1 提取后 | 约 680 行 | -94 行发送方法体, -14 行信号连接, +15 行委托代码, +10 行 SendController 集成 |
| R2 提取后 | 约 490 行 | -181 行连接方法体, -10 行状态栏更新, +15 行委托代码, +5 行 ConnectionController 集成 |
| **目标** | **< 500 行** | 满足 CLAUDE.md 4.6 约束 |

### 跨模块影响评估

- **R1 独立**: SendController 提取不涉及其他 Controller 变更。SendHistory 和 TimedSender 的公共接口保持不变。
- **R2 独立**: ConnectionController 提取不涉及其他 Controller 变更。ConnectionManager、IConnection 的公共接口保持不变。
- **R1+R2 协作**: MainWindow 在 connectSignals() 中通过信号连接 SendController 和 ConnectionController，两个 Controller 之间无直接依赖。
- **R3 嵌入 R1**: sendAndRecord 修复在 SendController 中完成，不影响其他模块。

---

## 验收标准总表

| 需求 | 编号 | 验收条件 | 度量方法 | 通过标准 |
|------|------|---------|---------|---------|
| R1 | R1-AC1 | SendController 编译通过 | cmake --build | 0 error |
| R1 | R1-AC2 | 文本模式发送正常 | 手动测试 | 终端显示已发送数据 |
| R1 | R1-AC3 | HEX 模式发送正常 | 手动测试 | 终端显示 HEX 数据 |
| R1 | R1-AC4 | HEX 无效输入错误态 | 手动测试 | 输入框变红 |
| R1 | R1-AC5 | 回车发送正常 | 手动测试 | 数据发出 |
| R1 | R1-AC6 | 快捷指令正常 | 手动测试 | 数据发出 |
| R1 | R1-AC7 | 定时发送正常 | 手动测试 | 按时发送 |
| R1 | R1-AC8 | 发送历史自动补全 | 手动测试 | 下拉匹配 |
| R1 | R1-AC9 | 发送历史记录正确 | 手动测试 | 去重正常 |
| R2 | R2-AC1 | ConnectionController 编译通过 | cmake --build | 0 error |
| R2 | R2-AC2 | 串口连接/断开 | 手动测试 | 状态正确 |
| R2 | R2-AC3 | TCP 客户端连接 | 手动测试 | 连接发起 |
| R2 | R2-AC4 | TCP 服务端连接 | 手动测试 | 服务端启动 |
| R2 | R2-AC5 | UDP 连接 | 手动测试 | Socket 创建 |
| R2 | R2-AC6 | 连接状态呼吸动画 | 手动测试 | 脉冲闪烁 |
| R2 | R2-AC7 | 连接成功自动切换终端 | 手动测试 | 面板切换 |
| R2 | R2-AC8 | 数据接收正常 | 手动测试 | 终端显示 |
| R2 | R2-AC9 | RX/TX 统计正确 | 手动测试 | 字节数正确 |
| R2 | R2-AC10 | 窗口关闭连接清理 | 手动测试 | 无崩溃 |
| R2 | R2-AC11 | OTA 连接同步 | 手动测试 | OTA 可用 |
| R2 | R2-AC12 | 设置保存/恢复正常 | 手动测试 | 配置持久化 |
| R3 | R3-AC1 | 未连接发送有提示 | 手动测试 | 状态栏消息 |
| R3 | R3-AC2 | write=0 有提示 | 模拟测试 | 缓冲区满提示 |
| R3 | R3-AC3 | write=-1 有提示 | 模拟测试 | 写入错误提示 |
| R3 | R3-AC4 | 失败时保留输入 | 手动测试 | 输入框不清空 |
| R3 | R3-AC5 | 正常发送不受影响 | 手动测试 | 正常工作 |
| -- | **AC-FINAL** | **MainWindow.cpp < 500 行** | **wc -l** | **< 500** |

---

## 实施优先级

| 顺序 | 需求 | 优先级 | 理由 |
|------|------|--------|------|
| 1 | **R1 (P0)** | SendController 提取 | 行数贡献最大且逻辑最内聚，优先完成可验证发送功能完整性 |
| 2 | **R3 (P0)** | sendAndRecord 修复 | 在 R1 的 SendController 中直接修复，不需要额外迭代 |
| 3 | **R2 (P0)** | ConnectionController 提取 | R1 完成后 MainWindow 行数已大幅减少，R2 完成后达到 500 行以下目标 |

**串行策略**: R1 和 R3 同步完成（同一文件），R2 在 R1 之后执行。原因: R2 的 ConnectionController 需要设置 SendController 的 currentConnection，依赖 R1 中 SendController 的 setCurrentConnection 接口已就绪。

---

## 验证度量指标

### 代码度量

| 度量项 | 度量方法 | 当前基线 | 目标值 |
|--------|---------|---------|--------|
| MainWindow.cpp 行数 | wc -l | 762 行 | < 500 行 |
| MainWindow.h 行数 | wc -l | 182 行 | < 200 行 |
| SendController.h 行数 | wc -l | 新建 | < 200 行 |
| SendController.cpp 行数 | wc -l | 新建 | < 500 行 |
| ConnectionController.h 行数 | wc -l | 新建 | < 200 行 |
| ConnectionController.cpp 行数 | wc -l | 新建 | < 500 行 |

### 架构度量

| 度量项 | 度量方法 | 目标值 |
|--------|---------|--------|
| MainWindow 中的槽方法数 | 声明计数 | 从 10 降至 4 (onDisplayModeChanged, onTimestampToggled, onClearTerminal, onExportData, onSearchRequested, onSearchCleared, onThemeChanged, onLanguageChanged) |
| MainWindow 中的业务方法数 | 声明计数 | 从 13 降至 7 |
| MainWindow 直接持有的 UI 组件数 | 成员计数 | 从 20+ 降至 10 以下 |
| Controller 间直接依赖 | 代码检查 | 0 (通过 MainWindow 中介) |

### 功能度量

| 度量项 | 度量方法 | 目标值 |
|--------|---------|--------|
| 全部发送功能回归 | 手动测试: 文本/HEX/快捷指令/定时/历史 | 100% 通过 |
| 全部连接功能回归 | 手动测试: 串口/TCP/UDP 连接断开 | 100% 通过 |
| 数据收发流程 | 手动测试: 连接 -> 发送 -> 接收 -> 断开 | 全流程通过 |
| 编译零错误零警告 | cmake --build | 0 error, 0 warning |
