/** @file MainWindow.h @brief 主窗口 -- 应用程序入口界面，组装所有面板和控制器。像嵌入式main.c一样简洁: 初始化对象->组装UI->连接信号/槽。业务逻辑委托给Controller/Manager */
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTreeView>
#include <QSplitter>
#include <QStatusBar>
#include <QLabel>
#include <functional>
#include "core/connect/ConnectionManager.h"
#include "core/theme/ThemeManager.h"
#include "terminal/widget/TerminalWidget.h"
#include "terminal/model/TerminalModel.h"
#include "serial/config/SerialConfigPanel.h"
#include "serial/commands/QuickCommandBar.h"
#include "serial/commands/SendHistory.h"
#include "serial/data/DataStatistics.h"
#include "utils/export/DataExporter.h"
#include "utils/log/DataLogger.h"
#include "core/connect/ConnectionController.h"
#include "core/recording/RecordingController.h"
#include "core/navigation/NavigationController.h"
#include "core/send/SendController.h"
#include "core/toolbar/ToolbarController.h"
#include "core/settings/SettingsController.h"
#include "core/terminal/TerminalController.h"
#include "core/panels/PanelManager.h"
#include "core/background/BackgroundWidget.h"
#include "core/background/BackgroundSettingsPopup.h"
#include "core/navigation/NavIndicatorWidget.h"
#include "core/settings/SessionManager.h"
#include "utils/settings/SettingsManager.h"
#include "terminal/search/TerminalSearchBar.h"
#include "terminal/layout/TerminalLayoutManager.h"
#include "protocol/parser/FrameParser.h"
#include "protocol/view/ProtocolView.h"
#include "protocol/editor/FrameVisualEditor.h"
#include "protocol/bridge/ProtocolBridgeManager.h"
#include "chart/widget/ChartWidget.h"
#include "ota/manager/OtaManager.h"
#include "ota/widget/OtaWidget.h"
#include "core/navigation/IconNavBar.h"
#include "core/widgets/CommandPalette.h"
#include "core/widgets/ScriptRecorder.h"
#include "core/layout/ResponsiveLayout.h"
#include "core/managers/ShortcutManager.h"
class StartupOptions;

/** @brief 主窗口 - EmbedDebug顶层窗口。职责: IDE布局构建+子Controller组装(依赖注入协调者)+信号/槽连接。中介者模式 */
class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr); ///< 构造(初始化子模块+组装UI)
    ~MainWindow() override = default;        ///< 析构(QObject父子树自动销毁)
    bool openPanelById(const QString& panelId); ///< 按稳定面板ID打开导航面板，供启动参数和外部入口复用
    bool applyStartupOptions(const StartupOptions& options); ///< 应用启动参数中的面板和档案路由

protected:
    void closeEvent(QCloseEvent* event) override; ///< 关闭事件(停动画/停录制/保存设置/关连接)

private slots:
    void onBgSettingsToggled();              ///< 切换背景设置弹出面板

private:
    void setupUI();                          ///< 构建完整UI布局(背景->分割器->导航树->面板栈->发送栏)
    QWidget* createNavigationArea();         ///< 创建左侧导航树区域
    QWidget* createContentArea();            ///< 创建右侧面板内容区域
    void setupStatusBar();                   ///< 创建并初始化状态栏
    void connectSignals();                   ///< 连接所有模块间信号/槽(调用8个子方法)
    void connectSerialSignals();             ///< 串口连接/断开/DTR/RTS/波特率信号路由
    void connectSerialDataFlow();            ///< 串口数据流+状态/错误信号路由
    void connectSerialSendSignals();         ///< 快捷指令/发送控制器信号路由
    void connectReconnectSignals();          ///< 自动重连状态指示信号路由
    void connectToolbarSignals();            ///< 工具栏/录制状态消息信号路由
    void connectSearchAndProtocolSignals();  ///< 搜索/协议桥/帧编辑/导航信号路由
    void connectPortWatchSignals();          ///< 串口热插拔状态栏通知
    void connectThemeSignals();              ///< 主题切换+Toast通知信号路由
    void connectOtaSignals();                ///< OTA传输Toast通知信号路由
    void connectBookmarkSignals();           ///< 书签面板CRUD信号路由
    void handleConnectionState(ConnectionState state, const QString& connName); ///< 处理连接状态变更
    void restoreUserSession(int lastPanel);  ///< 从磁盘恢复用户偏好
    void openQuickConnectionDialog();        ///< Ctrl+N 新建连接入口，弹出快速连接对话框
    void openTerminalSearch();               ///< Ctrl+F 搜索入口，切到终端并聚焦搜索栏
    void registerShortcuts();                ///< 注册全局快捷键管理器(搜索/命令面板/录制/清屏/发送/保存)
    void setupResponsiveLayout();            ///< 初始化响应式布局(断点系统+导航树自动折叠)

    // ---- 核心组件 ----
    ConnectionManager* m_connManager;          ///< 连接管理器(IConnection生命周期)
    ConnectionController* m_connController;    ///< 连接控制器(创建/断开连接)
    TerminalModel* m_terminalModel;            ///< 终端数据模型(收发数据+RX/TX计数)
    // ---- 业务组件 ----
    SendHistory* m_sendHistory;                ///< 发送历史管理器(自动补全)
    DataExporter* m_dataExporter;              ///< 数据导出器(TXT/CSV/BIN)
    DataLogger* m_dataLogger;                  ///< 数据日志记录器(录制/回放.edl)
    RecordingController* m_recordingController; ///< 录制/回放控制器
    SendController* m_sendController;          ///< 发送控制器(发送栏UI+HEX解析)
    // ---- UI组件 - 布局 ----
    QSplitter* m_mainSplitter;                 ///< 主水平分割器(导航树+面板区)
    QTreeView* m_navTree;                      ///< 左侧导航树

    /** @brief 导航树选中滑动指示器，在 navTree 左侧绘制动画 accent 色竖线 */
    NavIndicatorWidget* m_navIndicator;

    /** @brief 右侧面板容器，内部包含 serialPanel 面板栈 */
    QWidget* m_rightPanel;

    // ---- 工具栏 ----
    ToolbarController* m_toolbarController;    ///< 工具栏控制器(显示模式/主题/语言/录制)
    // ---- 面板管理 ----
    PanelManager* m_panelManager;              ///< 面板管理器(创建和管理所有功能面板)
    // ---- 终端布局 ----
    TerminalLayoutManager* m_layoutManager;    ///< 终端布局管理器(混合/左右/上下分栏)
    // ---- 协议解析 ----
    FrameParser* m_frameParser;                ///< 帧解析器(状态机解析字节流)
    ProtocolBridgeManager* m_protocolBridgeMgr; ///< 协议桥管理器(分发到ProtocolView+ChartWidget)
    // ---- OTA升级 ----
    OtaManager* m_otaManager;                  ///< OTA管理器(协调X/Y/ZModem传输)
    // ---- 状态栏 ----
    QLabel* m_connStatusLbl;                   ///< 连接状态标签
    // ---- 子控制器 ----
    NavigationController* m_navController;     ///< 导航控制器(面板切换+呼吸动画)
    SettingsController* m_settingsController;  ///< 设置控制器(配置持久化)
    TerminalController* m_terminalController;  ///< 终端控制器(显示/搜索/导出)
    // ---- 背景组件 ----
    BackgroundWidget* m_backgroundWidget;      ///< 背景层控件(模糊+涟漪特效)
    BackgroundSettingsPopup* m_bgSettingsPopup; ///< 背景设置弹出面板
    // ---- 会话管理 ----
    SessionManager* m_sessionManager;          ///< 会话管理器(窗口/配置保存恢复)
    // ---- 命令面板 ----
    CommandPalette* m_commandPalette = nullptr; ///< 命令面板(Ctrl+P快速导航)
    // ---- 脚本录制 ----
    ScriptRecorder* m_scriptRecorder = nullptr; ///< 脚本录制器(录制/回放发送序列)
    // ---- 图标导航栏(feature flag) ----
    IconNavBar* m_iconNavBar = nullptr;        ///< 图标导航栏(三栏布局, 默认关闭)
    bool m_useIconNavBar = false;              ///< 功能开关: "ui/iconNavBar" 配置项
    // ---- 响应式布局 ----
    ResponsiveLayout* m_responsiveLayout = nullptr; ///< 响应式布局(断点系统)
    // ---- 快捷键管理 ----
    ShortcutManager* m_shortcutManager = nullptr;   ///< 全局快捷键管理器(统一注册/绑定)
};

#endif // MAINWINDOW_H
