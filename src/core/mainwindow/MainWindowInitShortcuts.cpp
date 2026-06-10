/**
 * @file MainWindowInitShortcuts.cpp
 * @brief 主窗口快捷键注册与响应式布局初始化
 *
 * 从MainWindowInit.cpp拆分而来，包含构造函数中的快捷键管理器和响应式布局设置逻辑:
 *   - ShortcutManager全局快捷键注册(Ctrl+F/Ctrl+P/Ctrl+Shift+R/Ctrl+L/Ctrl+Enter/Ctrl+S/Ctrl+O/Ctrl+N/Ctrl+W)
 *   - ResponsiveLayout断点切换(移动端/平板/桌面/宽屏)与导航树自动折叠
 *   - 终端/发送区控件上下文映射注册
 *
 * 构造函数主体保留在MainWindowInit.cpp中。
 */

#include "core/mainwindow/MainWindow.h"
#include "core/mainwindow/MainWindowLayoutState.h"
#include "shared/AppConstants.h"

/** @brief 注册全局快捷键管理器，包含搜索/命令面板/录制/清屏/发送/保存/工程/连接/关闭快捷键 @param shortcutManager 快捷键管理器实例 @param commandPalette 命令面板实例 */
void MainWindow::registerShortcuts()
{
    m_shortcutManager = &ShortcutManager::instance();
    m_shortcutManager->registerShortcut(
        "search.find", QKeySequence("Ctrl+F"),
        this, [this]() { openTerminalSearch(); },
        tr("搜索"), ShortcutContext::Global);
    m_shortcutManager->registerShortcut(
        "nav.commandPalette", QKeySequence("Ctrl+P"),
        this, [this]() { m_commandPalette->showPalette(); },
        tr("命令面板"), ShortcutContext::Global);
    m_shortcutManager->registerShortcut(
        "script.recordToggle", QKeySequence("Ctrl+Shift+R"),
        this, [this]() {
            if (m_scriptRecorder->isRecording()) {
                m_scriptRecorder->stopRecording();
            } else {
                m_scriptRecorder->startRecording();
            }
        }, tr("录制脚本"), ShortcutContext::Global);
    m_shortcutManager->registerShortcut(
        "terminal.clear", QKeySequence("Ctrl+L"),
        this, [this]() { m_terminalController->onClearTerminal(); },
        tr("清空终端"), ShortcutContext::Terminal);
    m_shortcutManager->registerShortcut(
        "send.execute", QKeySequence("Ctrl+Enter"),
        this, [this]() { m_sendController->onQuickCommand(QByteArray()); },
        tr("发送数据"), ShortcutContext::SendArea);
    m_shortcutManager->registerShortcut(
        "project.save", QKeySequence("Ctrl+S"),
        this, [this]() { m_sessionManager->saveSession(); },
        tr("保存工程"), ShortcutContext::Global);
    // 以下快捷键已注册但回调为空，待后续模块实现后绑定
    m_shortcutManager->registerShortcut(
        "project.open", QKeySequence("Ctrl+O"),
        this, nullptr, tr("打开工程"), ShortcutContext::Global);
    m_shortcutManager->registerShortcut(
        "connection.new", QKeySequence("Ctrl+N"),
        this, [this]() { openQuickConnectionDialog(); },
        tr("新建连接"), ShortcutContext::Global);
    m_shortcutManager->registerShortcut(
        "tab.close", QKeySequence("Ctrl+W"),
        this, nullptr, tr("关闭标签页"), ShortcutContext::Global);

    // 加载用户自定义快捷键绑定(覆盖默认值)
    m_shortcutManager->loadCustomBindings();

    // 注册终端/发送区控件的上下文映射(焦点变化时自动切换快捷键上下文)
    if (m_panelManager->terminal()) {
        m_shortcutManager->registerContextWidget(
            m_panelManager->terminal(), ShortcutContext::Terminal);
    }
}

/** @brief 打开终端搜索，确保从任意面板触发时都回到终端上下文 */
void MainWindow::openTerminalSearch()
{
    if (m_navController && m_panelManager && m_panelManager->terminal()) {
        m_navController->switchToPanel(m_panelManager->terminal());
    }

    if (m_panelManager && m_panelManager->searchBar()) {
        m_panelManager->searchBar()->activate();
    }
}

/** @brief 初始化响应式布局，监听窗口宽度变化自动切换断点(移动端/平板/桌面/宽屏) */
void MainWindow::setupResponsiveLayout()
{
    m_responsiveLayout = new ResponsiveLayout(this);
    m_responsiveLayout->watchWindow(this);
    connect(m_responsiveLayout, &ResponsiveLayout::breakpointChanged,
            this, [this](ResponsiveLayout::Breakpoint bp, ResponsiveLayout::Breakpoint /*oldBp*/) {
        if (bp == ResponsiveLayout::Breakpoint::Mobile) {
            if (m_iconNavBar) { m_iconNavBar->hide(); }
            statusBar()->showMessage(tr("已切换到移动端布局"), 2000);
            m_panelManager->setCompactMode(true);
        } else if (bp == ResponsiveLayout::Breakpoint::Tablet) {
            if (m_iconNavBar && m_useIconNavBar) { m_iconNavBar->show(); }
            statusBar()->showMessage(tr("已切换到平板布局"), 2000);
            m_panelManager->setCompactMode(false);
        } else if (bp == ResponsiveLayout::Breakpoint::Desktop) {
            if (m_mainSplitter && navTreeIsCollapsedInSplitterSizes(m_mainSplitter->sizes(), m_useIconNavBar)) {
                int navWidth = SettingsManager::instance().get("layout/navTreeWidth").toInt();
                if (navWidth <= 0) navWidth = 200;
                m_mainSplitter->setSizes(restoredNavigationSplitterSizes(navWidth, width(), m_useIconNavBar));
            }
            if (m_iconNavBar && m_useIconNavBar) { m_iconNavBar->show(); }
            statusBar()->showMessage(tr("已切换到桌面布局"), 2000);
            m_panelManager->setCompactMode(false);
        } else if (bp == ResponsiveLayout::Breakpoint::Wide) {
            if (m_mainSplitter && navTreeIsCollapsedInSplitterSizes(m_mainSplitter->sizes(), m_useIconNavBar)) {
                int navWidth = SettingsManager::instance().get("layout/navTreeWidth").toInt();
                if (navWidth <= 0) navWidth = 240;
                m_mainSplitter->setSizes(restoredNavigationSplitterSizes(navWidth, width(), m_useIconNavBar));
            }
            if (m_iconNavBar && m_useIconNavBar) { m_iconNavBar->show(); }
            statusBar()->showMessage(tr("已切换到宽屏布局"), 2000);
            m_panelManager->setCompactMode(false);
        }
    });

    // 响应式布局: 导航树自动折叠（< 900px）
    connect(m_responsiveLayout, &ResponsiveLayout::navTreeAutoCollapse,
            this, [this](bool collapsed) {
        int navWidth = SettingsManager::instance().get("layout/navTreeWidth").toInt();
        if (navWidth <= 0) navWidth = 200;
        m_navController->onBreakpointNavCollapse(collapsed, m_mainSplitter, navWidth);
    });
}
