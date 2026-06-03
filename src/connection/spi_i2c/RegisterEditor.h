/**
 * @file RegisterEditor.h
 * @brief 寄存器编辑器 - 十六进制寄存器读写编辑UI
 *
 * 职责:
 *   1. 提供地址和数据输入控件
 *   2. 支持寄存器读取和写入操作
 *   3. 显示操作日志，格式化十六进制显示
 *   4. 通过I2cConnection/SpiConnection执行实际读写
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
#include <QLabel>
#include <QVBoxLayout>
#include <QByteArray>

class IConnection;

/**
 * @brief 寄存器编辑器UI
 *
 * 提供地址输入、数据输入、读/写按钮和操作日志。
 * 支持十六进制格式的寄存器地址和数据输入。
 * 日志格式: "[RW] ADDR: 0x42 → DATA: 0x1A 0x2B"
 */
class RegisterEditor : public QWidget {
    Q_OBJECT

public:
    /** @brief 构造函数
     * @param parent 父控件
     */
    explicit RegisterEditor(QWidget* parent = nullptr);

    /**
     * @brief 设置底层连接(I2C或SPI)
     * @param connection IConnection实例(需为I2C或SPI)
     */
    void setConnection(IConnection* connection);

    /**
     * @brief 设置I2C设备地址(7位)
     * @param addr 设备地址(0x00-0x7F)
     */
    void setDeviceAddress(int addr) { m_deviceAddress = addr; }

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

    /**
     * @brief 设置读取长度
     * @param length 默认读取字节数
     */
    void setReadLength(int length);

    /** @brief 获取读操作次数 */
    int readCount() const;

    /** @brief 获取写操作次数 */
    int writeCount() const;

    /** @brief 导出操作日志为文本 */
    QString exportLog() const;

signals:
    /** @brief 寄存器读取完成信号
     * @param address 寄存器地址
     * @param data 读取到的数据
     */
    void registerReadComplete(int address, const QByteArray& data);

    /** @brief 寄存器写入完成信号
     * @param address 寄存器地址
     * @param success 是否成功
     */
    void registerWriteComplete(int address, bool success);

private slots:
    /** @brief 读取按钮点击 */
    void onReadClicked();

    /** @brief 写入按钮点击 */
    void onWriteClicked();

    /** @brief 清空日志按钮点击 */
    void onClearLogClicked();

private:
    /** @brief 初始化UI布局 */
    void setupUi();

    /** @brief 初始化信号连接 */
    void setupConnections();

    /** @brief 追加日志
     * @param msg 日志消息
     * @param isTx true=发送(蓝色)，false=接收(绿色)
     */
    void appendLog(const QString& msg, bool isTx = true);

    /** @brief 格式化字节数组为十六进制字符串 */
    static QString formatHex(const QByteArray& data);

    IConnection* m_connection = nullptr;            ///< 底层连接实例
    int m_readLength = 1;                            ///< 默认读取长度
    int m_deviceAddress = 0x00;                      ///< I2C设备7位地址
    int m_readCount = 0;                             ///< 读操作计数
    int m_writeCount = 0;                            ///< 写操作计数

    // ---- UI控件 ----
    QSpinBox* m_addrSpin = nullptr;                 ///< 寄存器地址输入
    QSpinBox* m_lengthSpin = nullptr;               ///< 读取长度输入
    QLineEdit* m_dataEdit = nullptr;                ///< 数据输入(十六进制)
    QPushButton* m_readBtn = nullptr;               ///< 读取按钮
    QPushButton* m_writeBtn = nullptr;              ///< 写入按钮
    QPushButton* m_clearLogBtn = nullptr;           ///< 清空日志按钮
    QTextEdit* m_log = nullptr;                     ///< 操作日志
};

#endif // REGISTEREDITOR_H
