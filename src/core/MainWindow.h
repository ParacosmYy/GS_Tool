#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTreeView>
#include <QSplitter>
#include <QStatusBar>
#include <QLabel>
#include <functional>
#include "ConnectionManager.h"
#include "ThemeManager.h"
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
#include "core/PanelManager.h"
#include "core/BackgroundWidget.h"
#include "core/BackgroundSettingsPopup.h"
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
#include "Constants.h"

/**
 * @brief 主窗口 - EmbedDebug 应用的顶层窗口
 *
 * 职责:
 *   1. 构建 IDE 风格布局: 左侧导航树 + 右侧功能面板
 *   2. 作为所有子 Controller/Manager 的组装点（依赖注入协调者）
 *   3. 连接各模块的信号/槽，完成跨模块协作
 *   4. 不包含业务逻辑本身，业务逻辑全部委托给各 Controller
 *
 * 设计模式: 中介者模式（Mediator）- 协调各 Controller 之间的交互
 *
 * 协作关系:
 *   - ConnectionController: 管理串口/网络连接的生命周期
 *   - SendController: 管理发送栏 UI 和数据发送
 *   - NavigationController: 管理导航树和面板切换动画
 *   - ToolbarController: 管理工具栏控件创建和事件转发
 *   - SettingsController: 管理设置的加载/保存
 *   - RecordingController: 管理数据录制和回放
 *   - BackgroundWidget: 提供磨砂玻璃背景和涟漪特效
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
    /**
     * @brief 终端显示模式切换（文本/HEX/混合/十进制）
     * @param index 下拉框选中索引: 0=文本, 1=HEX, 2=混合, 3=十进制
     */
    void onDisplayModeChanged(int index);

    /**
     * @brief 时间戳显示开关切换
     * @param checked true=显示时间戳, false=不显示
     */
    void onTimestampToggled(bool checked);

    /** @brief 清空终端内容和数据统计，重置 RX/TX 字节计数 */
    void onClearTerminal();

    /** @brief 导出终端数据到文件（支持 TXT/CSV/BIN 格式） */
    void onExportData();

    /**
     * @brief 终端搜索请求处理
     * @param pattern 搜索模式字符串
     * @param regex 是否使用正则表达式
     * @param hex 是否为 HEX 模式搜索
     */
    void onSearchRequested(const QString& pattern, bool regex, bool hex);

    /** @brief 清除终端搜索高亮 */
    void onSearchCleared();

    /** @brief 切换背景设置弹出面板的显示/隐藏 */
    void onBgSettingsToggled();

    /**
     * @brief 终端布局模式切换（混合/左右分栏/上下分栏）
     * @param index 下拉框选中索引: 0=混合, 1=左右分栏, 2=上下分栏
     */
    void onTerminalLayoutChanged(int index);

private:
    /** @brief 构建完整的 UI 布局（背景层→分割器→导航树→面板栈→发送栏） */
    void setupUI();

    /** @brief 创建并初始化状态栏（连接状态、RX/TX 字节数） */
    void setupStatusBar();

    /**
     * @brief 连接所有模块间的信号/槽
     * 包括: 串口连接/断开、数据收发、工具栏事件、录制/回放、搜索、
     *        协议解析、帧编辑器、导航树面板切换
     */
    void connectSignals();

    /** @brief 刷新状态栏中的 RX/TX 字节数显示 */
    void updateStatusBar();

    /** @brief 定时刷新数据统计面板的 RX/TX 累计字节 */
    void updateDataStatistics();

    // ==================== 核心组件 ====================

    /** @brief 连接管理器，管理所有 IConnection 实例的生命周期 */
    ConnectionManager* m_connManager;

    /** @brief 连接控制器，负责创建/断开连接并分发连接状态 */
    ConnectionController* m_connController;

    /** @brief 终端数据模型，存储收发数据行和 RX/TX 字节计数 */
    TerminalModel* m_terminalModel;

    // ==================== 业务组件 ====================

    /** @brief 发送历史管理器，维护最近发送记录用于自动补全 */
    SendHistory* m_sendHistory;

    /** @brief 数据导出器，支持 TXT/CSV/BIN 格式的流式批量导出 */
    DataExporter* m_dataExporter;

    /** @brief 数据日志记录器，负责录制/回放 .edl 日志文件 */
    DataLogger* m_dataLogger;

    /** @brief 录制/回放控制器，管理录制和回放按钮的交互 */
    RecordingController* m_recordingController;

    /** @brief 发送控制器，管理发送栏 UI、输入解析（文本/HEX）、连接写入、终端记录 */
    SendController* m_sendController;

    // ==================== UI组件 - 布局 ====================

    /** @brief 主水平分割器，左导航树 + 右面板区 */
    QSplitter* m_mainSplitter;

    /** @brief 左侧导航树控件，由 NavigationController 构建数据模型 */
    QTreeView* m_navTree;

    /** @brief 右侧面板容器，内部包含 serialPanel 面板栈 */
    QWidget* m_rightPanel;

    // ==================== UI组件 - 工具栏 ====================

    /** @brief 工具栏控制器，创建和管理所有工具栏控件（显示模式/主题/语言/录制等） */
    ToolbarController* m_toolbarController;

    // ==================== 面板管理 ====================

    /** @brief 面板管理器，统一创建和管理所有功能面板（串口配置/终端/统计/协议/波形图/OTA等） */
    PanelManager* m_panelManager;

    // ==================== UI组件 - 终端布局 ====================

    /** @brief 终端布局管理器，管理混合/左右分栏/上下分栏三种布局模式切换 */
    TerminalLayoutManager* m_layoutManager;

    // ==================== UI组件 - 协议解析 ====================

    /** @brief 帧解析器，根据 FrameDefinition 定义的状态机解析原始字节流 */
    FrameParser* m_frameParser;

    /** @brief 协议桥管理器，将帧解析器的输出分发到 ProtocolView 和 ChartWidget */
    ProtocolBridgeManager* m_protocolBridgeMgr;

    // ==================== UI组件 - OTA升级 ====================

    /** @brief OTA 管理器，协调 XModem/YModem/ZModem 传输协议 */
    OtaManager* m_otaManager;

    // ==================== UI组件 - 状态栏 ====================

    /** @brief 连接状态标签，显示"未连接"/"已连接: xxx"/"连接中..."等状态 */
    QLabel* m_connStatusLbl;

    /** @brief 接收字节计数标签，格式: "RX: xxx B/KB/MB" */
    QLabel* m_rxBytesLbl;

    /** @brief 发送字节计数标签，格式: "TX: xxx B/KB/MB" */
    QLabel* m_txBytesLbl;

    // ==================== 定时器 ====================

    /** @brief 统计刷新定时器，每 500ms 触发一次 updateDataStatistics() */
    QTimer* m_statsTimer;

    // ==================== 子控制器 ====================

    /** @brief 导航控制器，负责导航树构建、面板切换动画（淡入淡出）和呼吸动画 */
    NavigationController* m_navController;

    /** @brief 设置控制器，负责窗口几何/主题/串口配置/语言的持久化加载与保存 */
    SettingsController* m_settingsController;

    // ==================== 背景组件 ====================

    /** @brief 背景层控件，提供自定义背景图、磨砂玻璃模糊、透明度调节和点击涟漪特效 */
    BackgroundWidget* m_backgroundWidget;

    /** @brief 背景设置弹出面板，浮动在工具栏下方，提供模糊半径/透明度/涟漪开关调节 */
    BackgroundSettingsPopup* m_bgSettingsPopup;
};

#endif // MAINWINDOW_H
