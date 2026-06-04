/**
 * @file MainWindowSignalConnect.cpp
 * @brief 主窗口信号/槽连接 - 串口连接/主题/发送/热插拔/重连信号路由
 *
 * 从 MainWindow.cpp 拆分，包含 connectSignals() 及以下子方法:
 *   connectSerialSignals()           - 串口连接/断开/DTR/RTS/波特率/数据/错误
 *   connectSerialDataFlow()          - 串口数据流 + 状态/错误信号连接
 *   connectReconnectSignals()        - 重连尝试/成功/失败状态
 *   connectSerialSendSignals()       - 快捷指令 → 发送控制器
 *   connectPortWatchSignals()        - 热插拔通知
 *   connectThemeSignals()            - 主题切换 + 连接Toast
 *
 * 另见 MainWindowPanelConnect.cpp:
 *   connectToolbarSignals()          - 工具栏/录制状态消息
 *   connectSearchAndProtocolSignals() - 搜索/协议桥/帧编辑/导航树
 *   connectOtaSignals()              - OTA传输通知
 *   connectBookmarkSignals()         - 书签面板CRUD
 */

#include "core/mainwindow/MainWindow.h"
#include "chart/model/ChartModel.h"
#include "serial/port/PortWatcher.h"
#include "core/widgets/ToastWidget.h"
#include "shared/AppConstants.h"
#include "connection/serial_port/SerialConnection.h"

/** @brief 连接所有模块间信号/槽（内部调用8个子方法按功能分组） */
void MainWindow::connectSignals()
{
    connectSerialSignals();
    connectReconnectSignals();
    connectSerialSendSignals();
    connectToolbarSignals();
    connectSearchAndProtocolSignals();
    connectPortWatchSignals();
    connectThemeSignals();
    connectOtaSignals();
    connectBookmarkSignals();
}

/** @brief 串口连接/断开/DTR/RTS/波特率/数据/错误信号路由 */
void MainWindow::connectSerialSignals()
{
    // ---- 串口连接/断开: 委托 ConnectionController 处理 ----
    connect(m_panelManager->serialConfig(), &SerialConfigPanel::connectRequested,
            this, [this]() {
        QVariantMap params;
        params["portName"] = m_panelManager->serialConfig()->currentPortData();
        params["baudRate"] = m_panelManager->serialConfig()->currentBaudRate();
        params["dataBits"] = m_panelManager->serialConfig()->currentDataBitsIndex() + 5;
        params["parity"] = m_panelManager->serialConfig()->currentParityIndex();
        params["stopBits"] = m_panelManager->serialConfig()->currentStopBitsIndex();
        params["flowControl"] = m_panelManager->serialConfig()->currentFlowControlIndex();
        params["dtr"] = m_panelManager->serialConfig()->dtrEnabled();
        params["rts"] = m_panelManager->serialConfig()->rtsEnabled();
        m_connController->connectSerial(params);
    });
    connect(m_panelManager->serialConfig(), &SerialConfigPanel::disconnectRequested,
            m_connController, &ConnectionController::disconnectCurrent);
    // DTR/RTS 线路控制信号直连
    connect(m_panelManager->serialConfig(), &SerialConfigPanel::dtrChanged,
            m_connController, &ConnectionController::setDtr);
    connect(m_panelManager->serialConfig(), &SerialConfigPanel::rtsChanged,
            m_connController, &ConnectionController::setRts);
    // Break 信号 → 连接控制器
    connect(m_panelManager->serialConfig(), &SerialConfigPanel::breakRequested,
            m_connController, &ConnectionController::sendBreak);
    // 自动重连开关 → 连接控制器
    connect(m_panelManager->serialConfig(), &SerialConfigPanel::autoReconnectToggled,
            this, [this](bool enabled, int intervalMs) {
        m_connController->enableAutoReconnect(enabled, intervalMs);
    });
    // 运行时波特率切换: 用户在连接状态下更改波特率
    connect(m_panelManager->serialConfig(), &SerialConfigPanel::baudRateChanged,
            this, [this](qint32 baud) {
        auto* conn = m_connController->currentConnection();
        if (conn && conn->type() == ConnectionType::Serial) {
            if (auto* serial = qobject_cast<SerialConnection*>(conn)) {
                serial->setBaudRate(baud);
                ToastWidget::show(this, tr("波特率已切换为 %1").arg(baud),
                                  ToastWidget::ToastType::Info);
            }
        }
    });
    // 连接状态变化 -> 更新 UI（状态栏文本、配置面板按钮状态、呼吸动画）
    connect(m_connController, &ConnectionController::connectionStateChanged,
            this, [this](ConnectionState state, const QString& connName) {
        handleConnectionState(state, connName);
    });
    // 通信错误计数更新 -> 数据统计面板（表现层直接连接，避免业务层依赖表现层）
    connect(m_connController, &ConnectionController::errorCountersUpdated,
            this, [this](int framing, int parity, int overrun) {
        m_panelManager->dataStats()->updateErrors(framing, parity, overrun);
    });
    // 连接健康状态 -> 数据统计面板（空闲超10秒时显示提示）
    connect(m_connController, &ConnectionController::connectionHealth,
            this, [this](bool alive, qint64 lastDataAgeMs) {
        m_panelManager->dataStats()->updateConnectionHealth(alive, lastDataAgeMs);
    });
    // 信号线状态变化 -> 更新串口配置面板LED指示灯
    connect(m_connController, &ConnectionController::pinoutSignalsChanged,
            m_panelManager->serialConfig(), &SerialConfigPanel::updatePinoutLeds);

    // 数据流 + 状态/错误通知
    connectSerialDataFlow();
}

/** @brief 串口数据流 + 状态/错误信号连接 */
void MainWindow::connectSerialDataFlow()
{
    // 接收数据 -> 终端模型 + 协议解析 + 日志记录
    connect(m_connController, &ConnectionController::dataReceived,
            this, [this](const QByteArray& data) {
        m_terminalModel->appendReceived(data);
        m_protocolBridgeMgr->feedData(data);
        m_dataLogger->logData(data, DataLogger::Direction::Received);
    });

    // 状态栏更新和连接失败通知
    connect(m_connController, &ConnectionController::statusBarUpdateRequested,
            m_terminalController, &TerminalController::updateStatusBar);
    // 连接失败 → 仅Toast通知（不再使用QMessageBox，避免自动重连时弹窗叠加阻塞UI）
    // 使用 showDebounced 防止同一错误消息在 3 秒内重复弹出
    connect(m_connController, &ConnectionController::connectionFailed,
            this, [this](const QString&, const QString& message) {
        ToastWidget::showDebounced(this, message, ToastWidget::ToastType::Error, 3000);
    });
}

/** @brief 自动重连状态指示信号路由（尝试/成功/失败→状态栏+Toast） */
void MainWindow::connectReconnectSignals()
{
    // 重连尝试中: 更新状态栏显示当前尝试次数
    connect(m_connController, &ConnectionController::reconnectAttempt,
            this, [this](int attempt, int maxRetries) {
        QString msg = maxRetries > 0
            ? tr("重连中... (第 %1/%2 次)").arg(attempt).arg(maxRetries)
            : tr("重连中... (第 %1 次)").arg(attempt);
        m_connStatusLbl->setText(msg);
        m_connStatusLbl->setProperty("state", "connecting");
        m_connStatusLbl->style()->unpolish(m_connStatusLbl);
        m_connStatusLbl->style()->polish(m_connStatusLbl);
    });
    // 重连进度通知(含指数退避间隔) -> Info类型吐司提示下次重连倒计时
    connect(m_connController, &ConnectionController::reconnectProgress,
            this, [this](int attempt, int maxRetries, int nextMs) {
        QString msg;
        if (maxRetries > 0) {
            msg = tr("正在重连... 第%1/%2次 (下次%3s后)")
                      .arg(attempt).arg(maxRetries).arg(nextMs / 1000.0, 0, 'f', 1);
        } else {
            msg = tr("正在重连... 第%1次 (下次%2s后)")
                      .arg(attempt).arg(nextMs / 1000.0, 0, 'f', 1);
        }
        ToastWidget::showDebounced(this, msg, ToastWidget::ToastType::Info, 2500);
    });
    // 重连成功: 状态栏由 handleConnectionState(Connected) 自动更新，此处仅显示 Toast
    connect(m_connController, &ConnectionController::reconnectSucceeded,
            this, [this](const QString& connName) {
        ToastWidget::show(this, tr("重连成功: %1").arg(connName),
                          ToastWidget::ToastType::Success);
    });
    // 重连最终失败: 更新状态栏 + 显示 Error 类型 Toast
    connect(m_connController, &ConnectionController::reconnectFailed,
            this, [this](const QString& reason) {
        m_connStatusLbl->setText(tr("重连失败"));
        m_connStatusLbl->setProperty("state", "error");
        m_connStatusLbl->style()->unpolish(m_connStatusLbl);
        m_connStatusLbl->style()->polish(m_connStatusLbl);
        ToastWidget::show(this, tr("重连失败: %1").arg(reason),
                          ToastWidget::ToastType::Error);
    });
}

/** @brief 快捷指令/发送控制器信号路由（发送→状态栏+防抖Toast） */
void MainWindow::connectSerialSendSignals()
{
    // 快捷指令 → 发送控制器
    connect(m_panelManager->quickCmdBar(), &QuickCommandBar::commandTriggered,
            m_sendController, &SendController::onQuickCommand);
    // 发送成功后更新状态栏
    connect(m_sendController, &SendController::dataSent,
            this, [this](qint64) { m_terminalController->updateStatusBar(); });
    // 发送状态消息显示到状态栏
    connect(m_sendController, &SendController::statusMessage,
            this, [this](const QString& msg) {
                statusBar()->showMessage(msg, 3000);
            });
    // 发送状态消息 → 防抖吐司通知（快速连续发送时可能频繁触发）
    connect(m_sendController, &SendController::statusMessage,
            this, [this](const QString& msg) {
                ToastWidget::showDebounced(this, msg);
            });
}

/** @brief 串口热插拔状态栏通知（新端口接入/端口拔出→状态栏提示） */
void MainWindow::connectPortWatchSignals()
{
    // ---- 串口热插拔状态栏通知 ----
    // 检测到新串口设备接入时，在状态栏显示提示信息
    connect(m_connController, &ConnectionController::portAdded,
            this, [this](const QString& portName) {
        statusBar()->showMessage(tr("检测到新端口: %1").arg(portName), 4000);
    });

    // 端口物理拔出时，除了 ConnectionController 自动断开连接外，
    // 额外在状态栏显示拔出提示（通过 PortWatcher 的 portRemoved 信号）
    connect(m_connController->portWatcher(), &PortWatcher::portRemoved,
            this, [this](const QString& portName) {
        statusBar()->showMessage(tr("端口已拔出: %1").arg(portName), 4000);
    });
}

/** @brief 主题切换信号连接 + 连接成功/断开/错误→Toast通知(含防抖策略) */
void MainWindow::connectThemeSignals()
{
    // ---- NavIndicatorWidget 主题刷新已在构造函数中连接，此处无需重复 ----
    // (移除重复连接，NavIndicatorWidget构造函数已连接themeChanged→updateThemeColor)

    /**
     * @name Toast 防抖策略
     *
     * 吐司通知的 debounce 策略:
     *
     * **使用 showDebounced()（防抖）** — 可能因自动重连、网络抖动等场景快速重复触发的信号:
     *   - connectionFailed: 自动重连失败时会连续触发，3 秒冷却避免重复弹窗
     *   - connectionError:  连接中途因错误断开，重连时可能重复触发，3 秒冷却
     *   - SendController::statusMessage: 快速连续发送时可能频繁触发，2 秒冷却（默认值）
     *   - OtaWidget::transferFailed: 传输失败重试时可能连续触发，3 秒冷却
     *
     * **使用 show()（无防抖）** — 确定性的一次性事件，不会在短时间内重复:
     *   - connectionSucceeded: 连接成功是一次性事件
     *   - connectionDisconnected: 用户主动断开是一次性事件
     *   - OtaWidget::transferStarted: 传输开始是一次性事件
     *   - OtaWidget::transferCompleted: 传输完成是一次性事件（含耗时/大小信息，不应丢弃）
     *   - RecordingController::statusMessage: 录制/回放状态切换是一次性事件
     *
     * 冷却键 = ToastType 编号 + "|" + 消息文本，同一消息+类型在冷却期内静默跳过。
     * 不同消息（如不同端口的错误）互不影响，各自独立计时。
     */
    ///@{

    // 连接成功时显示 Success 类型吐司（一次性事件，无需防抖）
    connect(m_connController, &ConnectionController::connectionSucceeded,
            this, [this](const QString& portName) {
        ToastWidget::show(this, tr("已连接: %1").arg(portName),
                          ToastWidget::ToastType::Success);
    });
    // 用户主动断开连接时显示 Info 类型吐司（一次性事件，无需防抖）
    connect(m_connController, &ConnectionController::connectionDisconnected,
            this, [this](const QString& portName) {
        ToastWidget::show(this, tr("已断开: %1").arg(portName),
                          ToastWidget::ToastType::Info);
    });
    // 连接因错误中断 → 防抖吐司（Error 类型）— 自动重连/网络抖动时可能快速重复触发，
    // 使用 showDebounced 防止同一端口+错误消息在 3 秒内重复弹出
    connect(m_connController, &ConnectionController::connectionError,
            this, [this](const QString& portName, const QString& error) {
        ToastWidget::showDebounced(this, tr("连接错误: %1\n%2").arg(portName, error),
                                   ToastWidget::ToastType::Error, 3000);
    });
    ///@}
}

// connectOtaSignals() 和 connectBookmarkSignals() 已移至 MainWindowPanelConnect.cpp
