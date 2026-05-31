/**
 * @file SendController.h
 * @brief 发送控制器 - 管理数据发送的完整生命周期
 *
 * 职责:
 *   1. 创建和管理发送栏 UI（输入框 + 模式切换 + 发送按钮 + 换行符选择）
 *   2. 解析用户输入（文本/HEX）并通过 IConnection 写入
 *   3. 记录发送数据到终端模型和日志
 *   4. 维护发送历史自动补全
 *   5. 管理定时发送器（TimedSender）
 *
 * 设计模式:
 *   - 策略模式: 文本/HEX 两种发送模式通过条件分支切换解析策略
 *   - 观察者模式: 通过 Qt 信号/槽通知发送状态
 *
 * 协作关系:
 *   - IConnection: 数据写入通道（由 ConnectionController 注入）
 *   - TerminalModel: 发送数据追加到终端显示
 *   - DataLogger: 发送数据记录到日志
 *   - SendHistory: 维护发送历史用于自动补全
 *   - TimedSender: 定时发送器，定时触发时通过 sendAndRecord 发出数据
 */

#ifndef SENDCONTROLLER_H
#define SENDCONTROLLER_H

#include <QObject>
#include <QStringListModel>
#include <QCompleter>

class QLineEdit;
class QPushButton;
class QComboBox;
class QCheckBox;
class QWidget;
class TerminalModel;
class DataLogger;
class SendHistory;
class TimedSender;
class IConnection;

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
    /**
     * @brief 构造发送控制器
     * @param model 终端数据模型，发送的数据追加到此模型
     * @param logger 数据日志记录器，记录发送的字节数据
     * @param history 发送历史管理器，提供自动补全数据源
     * @param parent 父对象
     */
    explicit SendController(TerminalModel* model, DataLogger* logger,
                            SendHistory* history, QObject* parent = nullptr);

    /** @brief 析构函数 */
    ~SendController() override;

    /**
     * @brief 创建发送输入区域并返回容器 widget
     *
     * 控件布局: [模式切换(文本/HEX)] [换行符选择] [输入框(带自动补全)] [发送按钮]
     * 自动补全数据源为 SendHistory 的最近发送记录
     *
     * @param parent 父 widget
     * @return 发送栏容器 widget，调用方将其加入布局
     */
    QWidget* createSendBar(QWidget* parent);

    /**
     * @brief 设置当前连接
     *
     * 由 ConnectionController 在连接/断开时调用。
     * 设置为 nullptr 后所有发送操作将返回失败并通过 statusMessage 通知用户。
     *
     * @param conn 新的连接实例，断开时传 nullptr
     */
    void setConnection(IConnection* conn);

    /**
     * @brief 获取定时发送器实例
     * 供外部配置界面（如定时发送设置面板）使用
     * @return 定时发送器指针
     */
    TimedSender* timedSender() const;

signals:
    /**
     * @brief 数据成功发送信号
     * @param bytes 成功写入的字节数
     */
    void dataSent(qint64 bytes);

    /**
     * @brief 状态消息信号（用于状态栏显示或通知提示）
     *
     * 发送时机:
     *   - 发送失败: 未连接、写入失败
     *   - HEX 解析错误: 无效的 HEX 格式
     *
     * @param msg 状态消息文本
     */
    void statusMessage(const QString& msg);

public slots:
    /**
     * @brief 快捷指令触发处理
     * 由 QuickCommandBar 的 commandTriggered 信号直接连接
     * @param data 预编码的原始字节数据（已是 HEX 或 UTF-8 编码后的结果）
     */
    void onQuickCommand(const QByteArray& data);

private slots:
    /**
     * @brief 发送按钮/回车触发的发送逻辑
     *
     * 流程:
     *   1. 前置检查（连接状态、输入非空）
     *   2. 根据发送模式解析输入（文本=toUtf8, HEX=HexConverter::fromHexString）
     *   3. HEX 解析失败时设置输入框错误样式（红色边框）
     *   4. 文本模式下追加换行符（\r\n/\n/\r）
     *   5. 调用 sendAndRecord() 写入数据
     *   6. 成功后记录历史并清空输入框
     */
    void onSendData();

private:
    /**
     * @brief 统一发送方法
     *
     * 写入连接 + 记录终端 + 记录日志，返回是否成功写入。
     * 失败时通过 statusMessage 信号通知用户具体的错误原因。
     *
     * @param data 待发送的原始字节数据
     * @return true=写入成功, false=写入失败或未连接
     */
    bool sendAndRecord(const QByteArray& data);

    // ==================== 核心依赖 ====================

    /** @brief 终端数据模型，发送数据追加到此模型用于终端显示 */
    TerminalModel* m_terminalModel;

    /** @brief 数据日志记录器，发送数据记录到 .edl 日志文件 */
    DataLogger* m_dataLogger;

    /** @brief 发送历史管理器，提供最近发送记录用于自动补全 */
    SendHistory* m_sendHistory;

    /** @brief 当前连接实例，由 ConnectionController 注入，断开时置 nullptr */
    IConnection* m_currentConn = nullptr;

    // ==================== 定时发送 ====================

    /** @brief 定时发送器，按设定间隔周期性发送数据 */
    TimedSender* m_timedSender;

    // ==================== UI 组件 - 发送区域 ====================

    /** @brief 发送输入框，支持文本和 HEX 输入，带发送历史自动补全 */
    QLineEdit* m_sendInput = nullptr;

    /** @brief 发送按钮，点击或回车触发 onSendData() */
    QPushButton* m_sendBtn = nullptr;

    /** @brief 发送模式切换下拉框: 0=文本, 1=HEX */
    QComboBox* m_sendModeCombo = nullptr;

    /** @brief 自动追加换行符选择: 0=无, 1=\r\n, 2=\n, 3=\r */
    QComboBox* m_newlineCombo = nullptr;

    // ==================== 自动补全 ====================

    /** @brief 发送历史的字符串列表模型，作为自动补全的数据源 */
    QStringListModel* m_sendCompleterModel = nullptr;

    /** @brief 输入框自动补全器，弹出匹配的历史发送记录 */
    QCompleter* m_sendCompleter = nullptr;
};

#endif // SENDCONTROLLER_H
