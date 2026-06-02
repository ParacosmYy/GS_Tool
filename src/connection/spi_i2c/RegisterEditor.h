/**
 * @file RegisterEditor.h
 * @brief 寄存器编辑器 - 十六进制寄存器读写编辑UI
 *
 * 职责:
 *   1. 提供地址和数据输入控件
 *   2. 支持寄存器读取和写入操作
 *   3. 显示操作日志
 *
 * 协作关系:
 *   - I2cConnection: I2C寄存器读写
 *   - SpiConnection: SPI寄存器读写
 */

#ifndef REGISTEREDITOR_H
#define REGISTEREDITOR_H

#include <QWidget>
#include <QSpinBox>
#include <QLineEdit>
#include <QPushButton>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QByteArray>

class IConnection;

/**
 * @brief 寄存器编辑器UI
 *
 * 提供地址输入、数据输入、读/写按钮和操作日志。
 * 支持十六进制格式的寄存器地址和数据输入。
 */
class RegisterEditor : public QWidget {
    Q_OBJECT

public:
    /** @brief 构造函数
     * @param parent 父控件
     */
    explicit RegisterEditor(QWidget* parent = nullptr);

    /**
     * @brief 设置底层连接
     * @param connection IConnection实例(需为I2C或SPI)
     */
    void setConnection(IConnection* connection);

    /**
     * @brief 读取指定地址的寄存器
     * @param address 寄存器地址
     */
    void readAddress(int address);

    /**
     * @brief 向指定地址写入数据
     * @param address 寄存器地址
     * @param data 待写入数据
     */
    void writeAddress(int address, const QByteArray& data);

private slots:
    /** @brief 读取按钮点击 */
    void onReadClicked();

    /** @brief 写入按钮点击 */
    void onWriteClicked();

private:
    /** @brief 初始化UI布局 */
    void setupUi();

    /** @brief 初始化信号连接 */
    void setupConnections();

    /** @brief 追加日志
     * @param msg 日志消息
     * @param isTx true=发送，false=接收
     */
    void appendLog(const QString& msg, bool isTx = true);

    IConnection* m_connection = nullptr;            ///< 底层连接实例

    // ---- UI控件 ----
    QSpinBox* m_addrSpin = nullptr;                 ///< 寄存器地址输入
    QLineEdit* m_dataEdit = nullptr;                ///< 数据输入(十六进制)
    QPushButton* m_readBtn = nullptr;               ///< 读取按钮
    QPushButton* m_writeBtn = nullptr;              ///< 写入按钮
    QTextEdit* m_log = nullptr;                     ///< 操作日志
};

#endif // REGISTEREDITOR_H
