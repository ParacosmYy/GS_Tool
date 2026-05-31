/**
 * @file MainWindowSignalConnect.cpp
 * @brief 主窗口信号/槽连接 - 所有模块间信号路由的集中连接点
 *
 * 本文件从 MainWindow.cpp 拆分而来，仅包含 connectSignals() 方法。
 * 将信号连接逻辑独立成文件，便于快速定位和维护模块间的通信关系。
 *
 * 信号流向（详见 connectSignals() 方法内部分组注释）:
 *   SerialConfigPanel → ConnectionController → MainWindow(状态更新)
 *   ConnectionController → TerminalModel(接收数据) + ProtocolBridgeMgr(协议解析)
 *   QuickCommandBar → SendController(发送数据)
 *   ToolbarController → TerminalController(显示模式/时间戳/清屏/导出)
 *   ToolbarController → SettingsController(主题/语言)
 *   RecordingController → MainWindow(回放数据写入终端)
 *   TerminalSearchBar → TerminalController(搜索高亮)
 *   FrameEditor → FrameParser(帧定义) + ChartWidget(波形配置)
 *   NavTree → NavigationController(面板切换)
 */

#include "MainWindow.h"
#include "chart/ChartModel.h"
#include "serial/PortWatcher.h"
#include <QMessageBox>

/**
 * @brief 连接所有模块间的信号/槽
 *
 * 信号流向:
 *   SerialConfigPanel → ConnectionController → MainWindow(状态更新)
 *   ConnectionController → TerminalModel(接收数据) + ProtocolBridgeMgr(协议解析)
 *   QuickCommandBar → SendController(发送数据)
 *   ToolbarController → TerminalController(显示模式/时间戳/清屏/导出)
 *   ToolbarController → SettingsController(主题/语言)
 *   RecordingController → MainWindow(回放数据写入终端)
 *   TerminalSearchBar → TerminalController(搜索高亮)
 *   FrameEditor → FrameParser(帧定义) + ChartWidget(波形配置)
 *   NavTree → NavigationController(面板切换)
 */
void MainWindow::connectSignals()
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

    // 连接状态变化 -> 更新 UI（状态栏文本、配置面板按钮状态、呼吸动画）
    connect(m_connController, &ConnectionController::connectionStateChanged,
            this, [this](ConnectionState state, const QString& connName) {
        handleConnectionState(state, connName);
    });

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
    connect(m_connController, &ConnectionController::connectionFailed,
            this, [this](const QString& title, const QString& message) {
        QMessageBox::warning(this, title, message);
    });

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

    // ---- 工具栏信号 → 委托给 TerminalController ----
    connect(m_toolbarController, &ToolbarController::displayModeChanged,
            m_terminalController, &TerminalController::onDisplayModeChanged);
    connect(m_toolbarController, &ToolbarController::timestampToggled,
            m_terminalController, &TerminalController::onTimestampToggled);
    connect(m_toolbarController, &ToolbarController::dirPrefixToggled,
            m_terminalController, &TerminalController::onDirPrefixToggled);
    connect(m_toolbarController, &ToolbarController::clearRequested,
            m_terminalController, &TerminalController::onClearTerminal);
    connect(m_toolbarController, &ToolbarController::exportRequested,
            this, [this]() { m_terminalController->onExportData(this); });
    connect(m_toolbarController, &ToolbarController::bgSettingsRequested,
            this, &MainWindow::onBgSettingsToggled);
    connect(m_toolbarController, &ToolbarController::themeChanged,
            m_settingsController, &SettingsController::onThemeChanged);
    connect(m_toolbarController, &ToolbarController::languageChanged,
            m_settingsController, &SettingsController::onLanguageChanged);
    connect(m_toolbarController, &ToolbarController::terminalLayoutChanged,
            m_terminalController, &TerminalController::onTerminalLayoutChanged);

    // TerminalController 状态消息 → 主窗口状态栏
    connect(m_terminalController, &TerminalController::statusMessage,
            this, [this](const QString& msg, int timeoutMs) {
                statusBar()->showMessage(msg, timeoutMs);
            });

    // 录制/回放状态消息 → 状态栏显示
    connect(m_recordingController, &RecordingController::statusMessage,
            this, [this](const QString& msg, int timeoutMs) {
                statusBar()->showMessage(msg, timeoutMs);
            });
    // 回放数据写入终端
    connect(m_recordingController, &RecordingController::playbackData,
            this, [this](const QByteArray& data, qint64 direction) {
                if (direction == 0) {
                    m_terminalModel->appendReceived(data);
                } else {
                    m_terminalModel->appendSent(data);
                }
                m_terminalController->updateStatusBar();
            });

    // 搜索栏 → TerminalController 搜索处理
    connect(m_panelManager->searchBar(), &TerminalSearchBar::searchRequested,
            m_terminalController, &TerminalController::onSearchRequested);
    connect(m_panelManager->searchBar(), &TerminalSearchBar::searchCleared,
            m_terminalController, &TerminalController::onSearchCleared);
    connect(m_panelManager->searchBar(), &TerminalSearchBar::closed, this, [this]() {
        m_terminalController->onSearchCleared();
    });

    // 搜索匹配结果 → 搜索栏显示匹配计数
    connect(m_panelManager->terminal(), &TerminalWidget::searchMatchesChanged,
            this, [this](int total, int current) {
                m_panelManager->searchBar()->setResultText(total == 0 ? QString() :
                    tr("%1/%2").arg(current + 1).arg(total));
            });

    // 协议桥 → 协议视图 + 波形图
    connect(m_protocolBridgeMgr, &ProtocolBridgeManager::frameParsed,
            m_panelManager->protocolView(), &ProtocolView::onFrameParsed);
    connect(m_protocolBridgeMgr, &ProtocolBridgeManager::frameError,
            m_panelManager->protocolView(), &ProtocolView::onFrameError);
    connect(m_protocolBridgeMgr, &ProtocolBridgeManager::frameParsed,
            m_panelManager->chartWidget()->model(), &ChartModel::onFrameParsed);

    // 帧编辑器 → 更新帧解析器定义 + 波形图通道配置
    connect(m_panelManager->frameEditor(), &FrameVisualEditor::definitionChanged,
            this, [this](const FrameDefinition& def) {
                m_frameParser->setDefinition(def);
                m_panelManager->chartWidget()->configureFromFrameDefinition(def);
            });

    // 导航树点击 → 面板切换
    connect(m_navTree, &QTreeView::clicked, this, [this](const QModelIndex& index) {
        QString text = index.data().toString();

        if (text == tr("数据导出")) { m_terminalController->onExportData(this); return; }
        if (text == tr("TCP客户端")) { m_connController->connectNetwork(ConnectionType::TcpClient); return; }
        if (text == tr("TCP服务端")) { m_connController->connectNetwork(ConnectionType::TcpServer); return; }
        if (text == tr("UDP")) { m_connController->connectNetwork(ConnectionType::Udp); return; }

        QWidget* target = m_navController->lookupPanel(text);
        if (!target) return;
        m_navController->switchToPanel(target);
    });

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
