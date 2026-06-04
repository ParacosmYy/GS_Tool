/**
 * @file BluetoothAtConsole.h
 * @brief 蓝牙AT指令控制台 — 提供AT指令发送和响应查看界面
 *
 * 职责: 提供AT指令输入、发送和响应展示功能，
 * 支持通过IConnection接口发送指令到蓝牙模块。
 * 包含HC-05/HC-06常用AT指令预设按钮面板。
 */
#ifndef BLUETOOTHATCONSOLE_H
#define BLUETOOTHATCONSOLE_H

#include <QWidget>
#include <QLineEdit>
#include <QTextEdit>
#include <QPushButton>
#include <QLabel>
#include <QList>

class IConnection;

/**
 * @brief 蓝牙AT指令控制台
 *
 * 顶部输出区 + 底部输入/发送栏 + 预设AT指令按钮面板。
 * 通过setConnection()绑定连接后可发送AT指令。
 */
class BluetoothAtConsole : public QWidget {
    Q_OBJECT

public:
    /**
     * @brief 构造AT控制台
     * @param parent 父控件
     */
    explicit BluetoothAtConsole(QWidget* parent = nullptr);

    /**
     * @brief 发送AT指令
     * @param command AT指令字符串(如 "AT+VERSION")
     */
    void sendCommand(const QString& command);

    /**
     * @brief 绑定连接实例
     * @param connection IConnection连接对象
     */
    void setConnection(IConnection* connection);

    /** @brief 获取累计发送AT命令次数 @return 发送总次数 */
    quint64 totalCommandsSent() const;

    /** @brief 获取累计接收响应次数 @return 接收总次数 */
    quint64 totalResponsesReceived() const;

    /** @brief 获取累计发送失败次数(连接不可用/写入失败) @return 错误计数 */
    quint64 totalSendErrors() const { return m_totalSendErrors; }

    /** @brief 获取累计发送字节总数(AT指令) @return 字节数 */
    quint64 totalBytesSent() const { return m_totalBytesSent; }

    /** @brief 获取累计超时次数 @return 超时总次数 */
    quint64 totalTimeouts() const { return m_totalTimeouts; }

    /** @brief 重置所有统计计数器 */
    void resetStatistics();

private slots:
    /** @brief 发送按钮点击处理 */
    void onSendClicked();

    /** @brief 预设指令按钮点击处理 */
    void onPresetClicked();

private:
    /** @brief AT指令输入框 */
    QLineEdit* m_cmdInput;

    /** @brief 响应输出区 */
    QTextEdit* m_output;

    /** @brief 发送按钮 */
    QPushButton* m_sendBtn;

    /** @brief 当前绑定的连接 */
    IConnection* m_connection = nullptr;

    /** @brief 预设AT指令按钮列表 */
    QList<QPushButton*> m_presetButtons;

    // ---- 统计计数器 ----
    quint64 m_totalCommandsSent = 0;            ///< 累计发送AT命令次数
    quint64 m_totalResponsesReceived = 0;       ///< 累计接收响应次数
    quint64 m_totalSendErrors = 0;              ///< 累计发送失败次数
    quint64 m_totalBytesSent = 0;               ///< 累计发送字节总数
    quint64 m_totalTimeouts = 0;                ///< 累计超时次数
};

#endif // BLUETOOTHATCONSOLE_H
