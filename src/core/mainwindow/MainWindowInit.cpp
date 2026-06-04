/**
 * @file MainWindowInit.cpp
 * @brief 主窗口构造函数与功能初始化
 *
 * 从 MainWindow.cpp 拆分而来，包含 MainWindow 构造函数的完整实现。
 * 构造函数内部按以下顺序执行初始化:
 *   1. 创建所有 Controller/Manager（通过构造函数依赖注入）
 *   2. 注入跨控制器依赖关系
 *   3. 构建 UI 布局和状态栏
 *   4. 连接所有信号/槽
 *   5. 构建导航树和面板映射
 *   6. 创建命令面板/脚本录制器
 *   7. 加载持久化设置
 *   8. 注册快捷键和响应式布局
 *
 * UI布局构建见 MainWindowSetupUI.cpp
 * 信号/槽连接见 MainWindowSignalConnect.cpp
 * 面板连接见 MainWindowPanelConnect.cpp
 * 生命周期与事件处理见 MainWindowLifecycle.cpp
 * 快捷键与响应式布局见 MainWindowInitShortcuts.cpp
 */

#include "core/mainwindow/MainWindow.h"
#include "serial/data/BookmarkWidget.h"
#include "shared/AppConstants.h"
#include <QVBoxLayout>
#include <QSplitter>
#include <QShortcut>
#include <QKeySequence>
#include <QDir>

/** @brief 构造函数 - 初始化所有子模块并组装主窗口 @param parent 父窗口 */
MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , m_connManager(new ConnectionManager(this))
    , m_connController(new ConnectionController(m_connManager, this))
    , m_terminalModel(new TerminalModel(this))
    , m_sendHistory(new SendHistory(this))
    , m_dataExporter(new DataExporter(this))
    , m_dataLogger(new DataLogger(this))
    , m_recordingController(new RecordingController(m_dataLogger, this))
    , m_sendController(new SendController(m_terminalModel, m_dataLogger, m_sendHistory, this))
    , m_frameParser(new FrameParser(this))
    , m_protocolBridgeMgr(new ProtocolBridgeManager(m_frameParser, this))
    , m_otaManager(new OtaManager(this))
    , m_panelManager(new PanelManager(this))
    , m_navController(new NavigationController(this))
    , m_toolbarController(new ToolbarController(m_recordingController, this))
    , m_settingsController(new SettingsController(this, this))
    , m_terminalController(new TerminalController(m_terminalModel, m_dataExporter, this))
{
    // 依赖注入: ConnectionController 需要在连接/断开时通知 SendController/OtaManager/RecordingController
    m_connController->setSendController(m_sendController);
    m_connController->setOtaManager(m_otaManager);
    m_connController->setRecordingController(m_recordingController);

    // 构建 UI 布局（背景层->分割器->导航树->面板栈->发送栏）
    setupUI();
    // 创建工具栏控件并添加到主窗口
    m_toolbarController->createToolbar(this);
    // 创建状态栏标签（连接状态、RX/TX 字节）
    setupStatusBar();

    // 背景设置弹出面板（浮动窗口，父级为背景层）
    m_bgSettingsPopup = new BackgroundSettingsPopup(m_backgroundWidget, this);

    // 从设置加载用户上次选择的自定义背景图
    QString customBg = SettingsManager::instance().get("background/customImagePath").toString();
    if (!customBg.isEmpty() && QFile::exists(customBg)) {
        m_backgroundWidget->setBackgroundImage(customBg);
    }

    // 注入依赖到 TerminalController（状态栏标签在 setupStatusBar() 中直接注入）
    m_terminalController->setLayoutManager(m_layoutManager);
    m_terminalController->setDataStatistics(m_panelManager->dataStats());
    m_terminalController->setMainTerminal(m_panelManager->terminal());

    // 连接所有模块间的信号/槽
    connectSignals();

    // 构建导航树模型: 通过 PanelManager 获取面板映射表
    m_navController->buildNavTree(m_navTree, m_panelManager->panelMappings());

    // 填充图标导航栏分类(从面板映射表提取去重分类)
    if (m_useIconNavBar && m_iconNavBar) {
        QVector<NavCategory> categories;
        const auto& mappings = m_panelManager->panelMappings();
        QMap<QString, NavCategory> seen;
        for (const auto& m : mappings) {
            if (!seen.contains(m.category)) {
                NavCategory cat;
                cat.id = QString::fromUtf8(m.category);
                cat.label = tr(m.category);
                cat.panelIds.append(QString::fromUtf8(m.name));
                seen.insert(m.category, cat);
            } else {
                seen[m.category].panelIds.append(QString::fromUtf8(m.name));
            }
        }
        categories = seen.values().toVector();
        m_iconNavBar->setCategories(categories);
        connect(m_iconNavBar, &IconNavBar::categoryClicked, this,
                [this](const QString& id) {
            Q_UNUSED(id);
        });
    }

    // 初始面板状态: 终端为默认可见面板（不触发动画）
    m_navController->setCurrentPanel(m_panelManager->terminal());

    // 命令面板 (Ctrl+P 快速导航 -- 已迁移到ShortcutManager统一注册)
    m_commandPalette = new CommandPalette(this);
    {
        QVector<CommandEntry> cmds;
        for (const auto& m : m_panelManager->panelMappings()) {
            CommandEntry e;
            e.id = "nav." + QString(m.name);
            e.category = tr("导航");
            e.label = tr(m.name);
            e.action = [this, m]() { m_navController->switchToPanel(m.widget); };
            cmds.append(e);
        }
        m_commandPalette->registerCommands(cmds);
    }

    // 脚本录制器 -- 从 PanelManager 获取已创建的面板实例
    m_scriptRecorder = m_panelManager->scriptRecorder();
    m_scriptRecorder->setObjectName("scriptRecorder");
    connect(m_scriptRecorder, &ScriptRecorder::playbackSendRequested,
            this, [this](const QString& data, bool isHex) {
        Q_UNUSED(isHex);
        m_sendController->onQuickCommand(data.toUtf8());
    });
    connect(m_scriptRecorder, &ScriptRecorder::recordingChanged,
            this, [this](bool recording) {
        statusBar()->showMessage(recording ? tr("脚本录制中...") : tr("录制已停止"), 3000);
    });

    // 设置 ThemeManager 主题切换淡入淡出动画目标（中央背景层）
    ThemeManager::instance().setTransitionWidget(m_backgroundWidget);

    // 注入 UI 引用到 SettingsController
    m_settingsController->setToolbarController(m_toolbarController);
    m_settingsController->setSerialConfigPanel(m_panelManager->serialConfig());

    // 创建会话管理器，注入依赖引用
    m_sessionManager = new SessionManager(this, this);
    m_sessionManager->setSettingsController(m_settingsController);
    m_sessionManager->setSerialConfigPanel(m_panelManager->serialConfig());

    // 从磁盘恢复上次保存的完整工作区
    int lastPanel = m_sessionManager->loadSession();
    restoreUserSession(lastPanel);

    // 设置窗口属性
    setWindowTitle(App::APP_NAME);
    resize(1200, 800);
    setMinimumSize(900, 600);

    // 注册全局快捷键管理器(Ctrl+F/Ctrl+P/Ctrl+Shift+R等)
    registerShortcuts();

    // 初始化响应式布局(断点系统+导航树自动折叠)
    setupResponsiveLayout();
}
