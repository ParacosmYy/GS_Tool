/**
 * @file MainWindowPanelConnect.cpp
 * @brief 主窗口面板/工具栏信号连接 - UI交互信号路由
 *
 * 从 MainWindowSignalConnect.cpp 拆分，包含面板交互相关的信号连接:
 *   connectToolbarSignals()          - 工具栏/录制状态消息
 *   connectSearchAndProtocolSignals() - 搜索/协议桥/帧编辑/导航
 *
 * 信号流:
 *   ToolbarController → TerminalController(显示/时间戳/清屏) + SettingsController(主题/语言)
 *   RecordingController → DataLogger(录制) + ToastWidget(反馈) + TerminalModel(回放)
 *   TerminalSearchBar → TerminalController(搜索)
 *   ProtocolBridgeManager → ProtocolView + ChartModel(协议数据)
 *   FrameVisualEditor → FrameParser(帧定义) + ChartWidget(波形通道)
 *   NavTree → NavigationController(面板切换动画)
 */

#include "core/mainwindow/MainWindow.h"
#include "chart/model/ChartModel.h"
#include "core/widgets/ToastWidget.h"
#include "shared/AppConstants.h"

/** @brief 工具栏和录制状态消息信号连接，包含ToolbarController到TerminalController/SettingsController的工具栏事件路由、状态消息到状态栏/吐司通知、回放数据写入终端 */
void MainWindow::connectToolbarSignals()
{
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
    // 录制/回放状态消息 → 吐司通知
    connect(m_recordingController, &RecordingController::statusMessage,
            this, [this](const QString& msg, int timeoutMs) {
                ToastWidget::show(this, msg, ToastWidget::ToastType::Info,
                                  timeoutMs > 0 ? timeoutMs : 3000);
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
}

/** @brief 搜索栏、协议桥、帧编辑器和导航树信号连接，包含搜索请求路由、协议数据分发、帧定义更新和面板切换动画 */
void MainWindow::connectSearchAndProtocolSignals()
{
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

    // 搜索历史变化 → 搜索栏更新补全列表
    connect(m_panelManager->terminal()->searchManager(),
            &TerminalSearchManager::searchHistoryChanged,
            m_panelManager->searchBar(),
            &TerminalSearchBar::updateSearchHistory);

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

        // 触发导航指示器滑动动画
        m_navIndicator->animateTo(index);

        if (text == tr("数据导出")) { m_terminalController->onExportData(this); return; }
        if (text == tr("TCP客户端")) { m_connController->connectNetwork(ConnectionType::TcpClient); return; }
        if (text == tr("TCP服务端")) { m_connController->connectNetwork(ConnectionType::TcpServer); return; }
        if (text == tr("UDP")) { m_connController->connectNetwork(ConnectionType::Udp); return; }

        QWidget* target = m_navController->lookupPanel(text);
        if (!target) return;
        m_navController->switchToPanel(target);
    });
}
