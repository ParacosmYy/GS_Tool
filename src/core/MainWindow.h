#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTreeView>
#include <QSplitter>
#include <QStackedWidget>
#include <QToolBar>
#include <QStatusBar>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QComboBox>
#include <QAction>
#include "ConnectionManager.h"
#include "ThemeManager.h"
#include "terminal/TerminalWidget.h"
#include "terminal/TerminalModel.h"
#include "serial/SerialConfigPanel.h"
#include "serial/QuickCommandBar.h"
#include "serial/TimedSender.h"
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

    // 发送数据
    void onSendData();

    // 快捷指令触发
    void onQuickCommand(const QByteArray& data);

    // 显示模式切换
    void onDisplayModeChanged(int index);

    // 时间戳开关
    void onTimestampToggled(bool checked);

    // 清空终端
    void onClearTerminal();

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

    // 核心组件
    ConnectionManager* m_connManager;

    // 当前活动的连接和模型
    IConnection* m_currentConn = nullptr;
    TerminalModel* m_terminalModel;

    // UI组件 - 布局
    QSplitter* m_mainSplitter;
    QTreeView* m_navTree;
    QStackedWidget* m_rightPanel;

    // UI组件 - 工具栏
    QToolBar* m_toolbar;
    QComboBox* m_displayModeCombo;
    QAction* m_timestampAction;
    QAction* m_clearAction;

    // UI组件 - 串口配置
    SerialConfigPanel* m_serialConfig;

    // UI组件 - 终端
    TerminalWidget* m_terminal;

    // UI组件 - 发送区域
    QLineEdit* m_sendInput;
    QPushButton* m_sendBtn;
    QPushButton* m_sendHexBtn;
    QComboBox* m_sendModeCombo;    // 文本/HEX切换

    // UI组件 - 快捷指令
    QuickCommandBar* m_quickCmdBar;

    // UI组件 - 状态栏
    QLabel* m_connStatusLbl;
    QLabel* m_rxBytesLbl;
    QLabel* m_txBytesLbl;

    // 定时发送器
    TimedSender* m_timedSender;
};

#endif // MAINWINDOW_H
