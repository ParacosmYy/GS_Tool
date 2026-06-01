/**
 * @file MainWindow.h
 * @brief 主窗口 — 应用程序入口界面，组装所有面板和控制器
 *
 * MainWindow像嵌入式main.c一样简洁：初始化对象→组装UI→连接信号/槽。
 * 所有业务逻辑委托给Controller/Manager类，MainWindow本身不包含业务逻辑。
 */
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTreeView>
#include <QSplitter>
#include <QStatusBar>
#include <QLabel>
#include <functional>
#include "core/ConnectionManager.h"
#include "core/ThemeManager.h"
#include "terminal/TerminalWidget.h"
#include "terminal/TerminalModel.h"
#include "serial/SerialConfigPanel.h"
#include "serial/QuickCommandBar.h"
#include "serial/SendHistory.h"
#include "serial/DataStatistics.h"
#include "utils/DataExporter.h"
#include "utils/DataLogger.h"
#include "core/ConnectionController.h"
#include "core/RecordingController.h"
#include "core/NavigationController.h"
#include "core/SendController.h"
#include "core/ToolbarController.h"
#include "core/SettingsController.h"
#include "core/TerminalController.h"
#include "core/PanelManager.h"
#include "core/BackgroundWidget.h"
#include "core/BackgroundSettingsPopup.h"
#include "core/NavIndicatorWidget.h"
#include "core/SessionManager.h"
#include "utils/SettingsManager.h"
#include "terminal/TerminalSearchBar.h"
#include "terminal/TerminalLayoutManager.h"
#include "protocol/FrameParser.h"
#include "protocol/ProtocolView.h"
#include "protocol/FrameVisualEditor.h"
#include "protocol/ProtocolBridgeManager.h"
#include "chart/ChartWidget.h"
#include "ota/OtaManager.h"
#include "ota/OtaWidget.h"
#include "core/Constants.h"

/**
 * @brief 主窗口 - EmbedDebug 应用的顶层窗口
 *
 * 职责: IDE布局构建 + 子Controller组装(依赖注入协调者) + 信号/槽连接
 * 不包含业务逻辑，所有逻辑委托给各 Controller。
 * 设计模式: 中介者模式(Mediator) - 协调各 Controller 交互
 * 协作: ConnectionController/SendController/NavigationController/
 *        ToolbarController/SettingsController/TerminalController/RecordingController
 */
class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    /**
     * @brief 构造主窗口，初始化所有子模块并组装 UI
     * @param parent 父窗口，默认无父级
     */
    explicit MainWindow(QWidget* parent = nullptr);

    /** @brief 析构，QObject 父子树自动销毁所有子组件 */
    ~MainWindow() override;

protected:
    /**
     * @brief 窗口关闭事件处理
     * 在关闭前执行清理工作: 停止呼吸动画、停止录制/回放、保存设置、关闭连接
     * @param event 关闭事件
     */
    void closeEvent(QCloseEvent* event) override;

private slots:
    /** @brief 切换背景设置弹出面板的显示/隐藏 */
    void onBgSettingsToggled();

private:
    /** @brief 构建完整的 UI 布局（背景层→分割器→导航树→面板栈→发送栏） */
    void setupUI();

    QWidget* createNavigationArea();  ///< 创建左侧导航树区域(导航树+选中滑动指示器)
    QWidget* createContentArea();     ///< 创建右侧面板内容区域(面板栈+终端+快捷指令+发送栏)

    /** @brief 创建并初始化状态栏（连接状态、RX/TX 字节数） */
    void setupStatusBar();

    void connectSignals();                 ///< 连接所有模块间信号/槽
    void connectSerialSignals();           ///< 串口连接/断开信号路由
    void connectSerialDataFlow();          ///< 串口数据流+状态/错误信号路由
    void connectSerialSendSignals();       ///< 快捷指令/发送控制器信号路由
    void connectReconnectSignals();        ///< 自动重连状态指示信号路由
    void connectToolbarSignals();          ///< 工具栏/录制状态消息信号路由
    void connectSearchAndProtocolSignals();///< 搜索/协议桥/帧编辑/导航信号路由
    void connectPortWatchSignals();        ///< 热插拔状态栏通知
    void connectThemeSignals();        ///< 主题切换 + Toast通知
    void connectOtaSignals();          ///< OTA传输Toast通知
    void connectBookmarkSignals();     ///< 书签面板信号路由

    /**
     * @brief 处理连接状态变更（更新状态栏、配置面板按钮、呼吸动画）
     * @param state 新的连接状态枚举
     * @param connName 连接名称
     */
    void handleConnectionState(ConnectionState state, const QString& connName);

    /** @brief 从磁盘恢复用户偏好（语言、主题、面板索引、统计定时器） */
    void restoreUserSession(int lastPanel);

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
};

#endif // MAINWINDOW_H
