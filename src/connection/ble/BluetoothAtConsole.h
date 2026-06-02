/**
 * @file BluetoothAtConsole.h
 * @brief 蓝牙AT指令控制台 — 提供AT指令发送和响应查看界面
 *
 * 职责: 提供AT指令输入、发送和响应展示功能，
 * 支持通过IConnection接口发送指令到蓝牙模块。
 */
#ifndef BLUETOOTHATCONSOLE_H
#define BLUETOOTHATCONSOLE_H

#include <QWidget>
#include <QLineEdit>
#include <QTextEdit>
#include <QPushButton>

class IConnection;

/**
 * @brief 蓝牙AT指令控制台
 *
 * 单行输入框 + 发送按钮 + 响应输出区。
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

private slots:
    /** @brief 发送按钮点击处理 */
    void onSendClicked();

private:
    /** @brief AT指令输入框 */
    QLineEdit* m_cmdInput;

    /** @brief 响应输出区 */
    QTextEdit* m_output;

    /** @brief 发送按钮 */
    QPushButton* m_sendBtn;

    /** @brief 当前绑定的连接 */
    IConnection* m_connection = nullptr;
};

#endif // BLUETOOTHATCONSOLE_H
