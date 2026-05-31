#ifndef SENDCONTROLLER_H
#define SENDCONTROLLER_H

#include <QObject>
#include <QStringListModel>
#include <QCompleter>

class QLineEdit;
class QPushButton;
class QComboBox;
class QWidget;
class TerminalModel;
class DataLogger;
class SendHistory;
class TimedSender;
class IConnection;

// 发送控制器 - 管理数据发送的完整生命周期
// 职责:
//   1. 创建和管理发送栏UI(输入框+模式切换+发送按钮)
//   2. 解析用户输入(文本/HEX)并通过IConnection写入
//   3. 记录发送数据到终端模型和日志
//   4. 维护发送历史自动补全
//   5. 管理定时发送器
class SendController : public QObject {
    Q_OBJECT

public:
    explicit SendController(TerminalModel* model, DataLogger* logger,
                            SendHistory* history, QObject* parent = nullptr);
    ~SendController() override;

    // 创建发送输入区域并返回容器widget，调用方将其加入布局
    QWidget* createSendBar(QWidget* parent);

    // 设置当前连接（由MainWindow在连接/断开时调用）
    void setConnection(IConnection* conn);

    // 获取定时发送器（供外部配置界面使用）
    TimedSender* timedSender() const;

signals:
    // 数据成功发送时发出（参数为写入字节数）
    void dataSent(qint64 bytes);

    // 状态消息（用于状态栏显示）
    void statusMessage(const QString& msg);

public slots:
    // 快捷指令触发（由QuickCommandBar信号直接连接）
    void onQuickCommand(const QByteArray& data);

private slots:
    // 发送按钮/回车触发的发送逻辑
    void onSendData();

private:
    // 统一发送方法: 写入连接 + 记录终端 + 日志，返回是否成功写入
    bool sendAndRecord(const QByteArray& data, bool isHex = false);

    // 核心依赖
    TerminalModel* m_terminalModel;
    DataLogger* m_dataLogger;
    SendHistory* m_sendHistory;
    IConnection* m_currentConn = nullptr;

    // 定时发送器
    TimedSender* m_timedSender;

    // UI组件 - 发送区域
    QLineEdit* m_sendInput = nullptr;
    QPushButton* m_sendBtn = nullptr;
    QComboBox* m_sendModeCombo = nullptr;

    // 发送历史自动补全
    QStringListModel* m_sendCompleterModel = nullptr;
    QCompleter* m_sendCompleter = nullptr;
};

#endif // SENDCONTROLLER_H
