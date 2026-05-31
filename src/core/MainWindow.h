#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTreeView>
#include <QSplitter>
#include <QToolBar>
#include <QStatusBar>
#include <QLabel>
#include <QComboBox>
#include <QAction>
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
#include "core/RecordingController.h"
#include "core/NavigationController.h"
#include "core/SendController.h"
#include "utils/SettingsManager.h"
#include "terminal/TerminalSearchBar.h"
#include "protocol/FrameParser.h"
#include "protocol/ProtocolView.h"
#include "protocol/FrameVisualEditor.h"
#include "chart/ChartWidget.h"
#include "ota/OtaManager.h"
#include "ota/OtaWidget.h"
#include "Constants.h"

// 主窗口 - 左侧导航树 + 右侧功能面板
// IDE风格布局，支持多连接Tab切换
class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override;

protected:
    void closeEvent(QCloseEvent* event) override;

private slots:
    // 串口连接/断开
    void onConnectSerial();
    void onDisconnectSerial();

    // 显示模式切换
    void onDisplayModeChanged(int index);

    // 时间戳开关
    void onTimestampToggled(bool checked);

    // 清空终端
    void onClearTerminal();

    // 导出数据
    void onExportData();

    // 搜索相关
    void onSearchRequested(const QString& pattern, bool regex, bool hex);
    void onSearchCleared();

    // 网络连接
    void onConnectNetwork(ConnectionType type);

    // 主题切换
    void onThemeChanged(int index);

    // 语言切换
    void onLanguageChanged(int index);

    // 连接状态变化
    void onConnectionStateChanged(ConnectionState state);

    // 接收到数据
    void onDataReceived(const QByteArray& data);

private:
    void setupUI();
    void setupToolbar();
    void setupStatusBar();
    void connectSignals();
    void updateStatusBar();
    void updateDataStatistics();
    void loadSettings();
    void saveSettings();

    // 核心组件
    ConnectionManager* m_connManager;

    // 当前活动的连接和模型
    IConnection* m_currentConn = nullptr;
    TerminalModel* m_terminalModel;

    // 业务组件
    SendHistory* m_sendHistory;
    DataExporter* m_dataExporter;
    DataLogger* m_dataLogger;
    RecordingController* m_recordingController;

    // 发送控制器（管理发送栏UI、发送逻辑、定时发送器）
    SendController* m_sendController;

    // UI组件 - 布局
    QSplitter* m_mainSplitter;
    QTreeView* m_navTree;
    QWidget* m_rightPanel;

    // UI组件 - 工具栏
    QToolBar* m_toolbar;
    QComboBox* m_displayModeCombo;
    QComboBox* m_themeCombo;
    QComboBox* m_langCombo;
    QAction* m_timestampAction;
    QAction* m_clearAction;
    QAction* m_exportAction;

    // UI组件 - 串口配置
    SerialConfigPanel* m_serialConfig;

    // UI组件 - 终端
    TerminalWidget* m_terminal;
    TerminalSearchBar* m_searchBar;

    // UI组件 - 快捷指令
    QuickCommandBar* m_quickCmdBar;

    // UI组件 - 数据统计
    DataStatistics* m_dataStats;

    // UI组件 - 协议解析
    ProtocolView* m_protocolView;
    FrameParser* m_frameParser;
    FrameVisualEditor* m_frameEditor;

    // UI组件 - 波形图
    ChartWidget* m_chartWidget;

    // UI组件 - OTA升级
    OtaManager* m_otaManager;
    OtaWidget* m_otaWidget;

    // UI组件 - 状态栏
    QLabel* m_connStatusLbl;
    QLabel* m_rxBytesLbl;
    QLabel* m_txBytesLbl;

    // 统计定时器
    QTimer* m_statsTimer;

    // 导航控制器（面板切换动画 + 呼吸动画 + 导航树构建）
    NavigationController* m_navController;
};

#endif // MAINWINDOW_H
