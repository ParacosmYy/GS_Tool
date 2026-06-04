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

// connectReconnectSignals/connectSerialSendSignals/connectPortWatchSignals/connectThemeSignals
// 已移至 MainWindowSignalConnectUI.cpp

// connectOtaSignals() 和 connectBookmarkSignals() 已移至 MainWindowPanelConnect.cpp
