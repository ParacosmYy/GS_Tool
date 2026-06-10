/**
 * @file MainWindowLifecycle.cpp
 * @brief 主窗口生命周期与事件处理方法
 *
 * 从 MainWindow.cpp 拆分而来，包含:
 *   restoreUserSession()       - 从磁盘恢复用户偏好（语言、主题、面板索引）
 *   handleConnectionState()    - 连接状态变更处理（状态栏/呼吸动画/面板切换）
 *   onBgSettingsToggled()      - 背景设置弹出面板显示/隐藏切换
 *   closeEvent()               - 窗口关闭事件（停止动画→停止录制→保存设置→关闭连接）
 *
 * UI布局构建见 MainWindowSetupUI.cpp
 * 信号/槽连接见 MainWindowSignalConnect.cpp
 * 面板连接见 MainWindowPanelConnect.cpp
 */

#include "core/mainwindow/MainWindow.h"
#include "core/mainwindow/MainWindowLayoutState.h"
#include "serial/commands/TimedSender.h"
#include <QCloseEvent>

/** @brief 从磁盘恢复用户偏好（语言、主题、面板索引）并启动统计定时器 */
void MainWindow::restoreUserSession(int lastPanel)
{
    if (m_toolbarController) {
        QString savedLang = SettingsManager::instance().loadLanguage();
        m_toolbarController->setCurrentLanguage(savedLang);
    }

    QString savedTheme = SettingsManager::instance().loadTheme();
    if (ThemeManager::instance().loadTheme(savedTheme)) {
        if (m_toolbarController) m_toolbarController->setCurrentTheme(savedTheme);
    }

    if (lastPanel >= 0) m_navController->restorePanelByIndex(lastPanel);

    m_terminalController->startStatsTimer();
}

/** @brief 处理连接状态变更(更新状态栏/配置面板/呼吸动画/自动切面板) @param state 连接状态枚举 @param connName 连接名称 */
void MainWindow::handleConnectionState(ConnectionState state, const QString& connName)
{
    const char* stateStr = "";
    switch (state) {
    case ConnectionState::Connected:
        m_connStatusLbl->setText(tr("已连接: %1").arg(connName));
        stateStr = "connected";
        m_panelManager->serialConfig()->setConnected(true);
        m_navController->switchToPanel(m_panelManager->terminal());
        m_navController->stopBreathingAnimation(m_connStatusLbl);
        break;
    case ConnectionState::Disconnected:
        m_connStatusLbl->setText(tr("未连接"));
        stateStr = "disconnected";
        m_panelManager->serialConfig()->setConnected(false);
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
        m_panelManager->serialConfig()->setConnected(false);
        m_navController->stopBreathingAnimation(m_connStatusLbl);
        break;
    default:
        stateStr = "unknown";
        break;
    }
    // 通过动态属性驱动 QSS 状态样式切换
    m_connStatusLbl->setProperty("state", stateStr);
    m_connStatusLbl->style()->unpolish(m_connStatusLbl);
    m_connStatusLbl->style()->polish(m_connStatusLbl);
}

/** @brief 切换背景设置弹出面板的显示/隐藏，面板定位在工具栏右下角 */
void MainWindow::onBgSettingsToggled()
{
    if (m_bgSettingsPopup->isVisible()) {
        m_bgSettingsPopup->hide();
    } else {
        m_bgSettingsPopup->syncFromWidget();
        QToolBar* tb = m_toolbarController->toolbar();
        QPoint pos = tb->mapToGlobal(QPoint(tb->width() - 270, tb->height() + 2));
        m_bgSettingsPopup->move(pos);
        m_bgSettingsPopup->show();
    }
}

/** @brief 窗口关闭事件处理(停止动画→停止录制→保存设置→关闭连接) @param event 关闭事件 */
void MainWindow::closeEvent(QCloseEvent* event)
{
    // 停止连接状态的呼吸动画
    m_navController->stopBreathingAnimation(m_connStatusLbl);

    // 停止录制/回放（防止后台线程写入已关闭文件）
    if (m_dataLogger->isRecording()) m_dataLogger->stopRecording();
    if (m_dataLogger->isPlaying()) m_dataLogger->stopPlayback();

    // 停止统计刷新定时器
    m_terminalController->stopStatsTimer();

    // 停止定时发送器（防止关闭后仍有挂起的发送）
    if (m_sendController && m_sendController->timedSender()) {
        m_sendController->timedSender()->stop();
    }

    // 保存当前面板索引（供下次恢复使用）
    if (m_navController) {
        int currentIdx = m_navController->currentPanelIndex();
        if (currentIdx >= 0) {
            m_settingsController->saveLastPanel(currentIdx);
        }
    }

    // 保存当前断点模式(下次启动时立即应用正确的布局)
    if (m_responsiveLayout) {
        SettingsManager::instance().set("layout/lastBreakpoint",
            static_cast<int>(m_responsiveLayout->currentBreakpoint()));
        m_responsiveLayout->saveLayoutConfig();
    }

    // 保存导航树宽度(Compact模式时保存上次展开宽度)
    if (m_mainSplitter) {
        auto sizes = m_mainSplitter->sizes();
        const int navTreeWidth = savedNavTreeWidthFromSplitterSizes(sizes, m_useIconNavBar);
        if (navTreeWidth > 0) {
            SettingsManager::instance().set("layout/navTreeWidth", navTreeWidth);
        }
    }

    // 通过会话管理器保存完整工作区到磁盘（窗口几何、串口配置、主题）
    m_sessionManager->saveSession();

    // 关闭所有活跃连接
    auto connections = m_connManager->connections();
    for (auto* conn : connections) {
        conn->close();
    }
    event->accept();
}
