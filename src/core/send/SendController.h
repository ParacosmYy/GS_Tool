/**
 * @file SendController.h
 * @brief 发送控制器 - 管理数据发送的完整生命周期
 *
 * 职责: UI创建、输入解析(文本/HEX)、IConnection写入、终端/日志记录、
 *       委托SendHistoryManager维护自动补全、管理TimedSender
 */

#ifndef SENDCONTROLLER_H
#define SENDCONTROLLER_H

#include <QObject>

class QLineEdit;
class QPushButton;
class QComboBox;
class QWidget;
class TerminalModel;
class DataLogger;
class SendHistory;
class TimedSender;
class IConnection;
class AnimatedButton;
class SendHistoryManager;

/**
 * @brief 发送控制器 - 管理数据发送的完整生命周期
 *
 * 核心发送流程:
 *   用户输入 → 解析(文本/HEX) → 追加换行符 → write() → 记录终端+日志
 *
 * 错误处理:
 *   - 未连接时发送 → statusMessage 通知用户
 *   - HEX 格式错误 → 输入框红色边框提示
 *   - 写入失败 → statusMessage 通知用户具体的错误信息
 */
class SendController : public QObject {
    Q_OBJECT

public:
    /** @brief 构造发送控制器 @param model 终端模型 @param logger 日志 @param history 历史存储 @param parent 父对象 */
    explicit SendController(TerminalModel* model, DataLogger* logger,
                            SendHistory* history, QObject* parent = nullptr);

    /** @brief 析构函数 */
    ~SendController() override = default;
    /** @brief 创建发送栏UI: [模式切换][换行符][输入框(带补全)][发送按钮]
     *  @param parent 父widget @return 发送栏容器widget */
    QWidget* createSendBar(QWidget* parent);
    /** @brief 设置当前连接，由ConnectionController注入，断开时传nullptr */
    void setConnection(IConnection* conn);
    /** @brief 获取定时发送器实例，供外部配置界面使用 */
    TimedSender* timedSender() const;

    // ---- 统计计数器接口 ----

    /** @brief 获取累计发送操作总次数 @return 发送次数 */
    quint64 totalSends() const;

    /** @brief 获取累计发送的字节总数 @return 字节数 */
    quint64 totalBytesSent() const;

    /** @brief 获取累计HEX模式发送次数 @return HEX发送次数 */
    quint64 totalHexSends() const;

    /** @brief 获取累计发送错误次数 @return 错误次数 */
    quint64 totalErrors() const;

    /** @brief 重置所有统计计数器(发送/字节/HEX/错误) */
    void resetSendStatistics();

signals:
    /** @brief 数据成功发送信号 @param bytes 成功写入的字节数 */
    void dataSent(qint64 bytes);
    /** @brief 状态消息信号(发送失败/HEX解析错误) @param msg 消息文本 */
    void statusMessage(const QString& msg);

public slots:
    /** @brief 快捷指令触发 @param data 预编码原始字节数据(HEX/UTF-8) */
    void onQuickCommand(const QByteArray& data);

private slots:
    /** @brief 发送按钮/回车: 前置检查→解析(文本/HEX)→追加换行→sendAndRecord→记录历史 */
    void onSendData();

private:
    /** @brief 统一发送: 写入连接+记录终端+记录日志 @param data 原始字节 @return 是否成功 */
    bool sendAndRecord(const QByteArray& data);

    // ==================== 核心依赖 ====================

    /** @brief 终端数据模型，发送数据追加到此模型用于终端显示 */
    TerminalModel* m_terminalModel;

    /** @brief 数据日志记录器，发送数据记录到 .edl 日志文件 */
    DataLogger* m_dataLogger;

    /** @brief 发送历史管理器（封装补全+频率聚合+键盘导航） */
    SendHistoryManager* m_historyManager;

    /** @brief 当前连接实例，由 ConnectionController 注入，断开时置 nullptr */
    IConnection* m_currentConn = nullptr;

    // ==================== 定时发送 ====================

    /** @brief 定时发送器，按设定间隔周期性发送数据 */
    TimedSender* m_timedSender;

    // ==================== UI 组件 - 发送区域 ====================

    /** @brief 发送输入框 */
    QLineEdit* m_sendInput = nullptr;

    /** @brief 发送按钮(带hover/press动画)，点击或回车触发 onSendData() */
    AnimatedButton* m_sendBtn = nullptr;

    /** @brief 发送模式切换下拉框: 0=文本, 1=HEX */
    QComboBox* m_sendModeCombo = nullptr;

    /** @brief 自动追加换行符选择: 0=无, 1=\r\n, 2=\n, 3=\r */
    QComboBox* m_newlineCombo = nullptr;

    // 统计计数器
    quint64 m_totalSends = 0;       ///< 累计发送操作总次数
    quint64 m_totalBytesSent = 0;   ///< 累计发送的字节总数
    quint64 m_totalHexSends = 0;    ///< 累计HEX模式发送次数
    quint64 m_totalErrors = 0;      ///< 累计发送错误次数
};

#endif // SENDCONTROLLER_H
