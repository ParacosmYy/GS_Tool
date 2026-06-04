/**
 * @file RegisterEditor.h
 * @brief 寄存器编辑器 - 十六进制寄存器读写编辑UI
 *
 * 职责: 地址和数据输入 / 寄存器读/写操作 / 操作日志(十六进制格式) / I2cConnection/SpiConnection读写
 */
#ifndef REGISTEREDITOR_H
#define REGISTEREDITOR_H

#include <QWidget>
#include <QSpinBox>
#include <QLineEdit>
#include <QPushButton>
#include <QTextEdit>
#include <QLabel>
#include <QByteArray>
#include <QSet>
#include <QElapsedTimer>

class IConnection;

/** @brief 寄存器编辑器UI — 地址输入、数据输入、读/写按钮和操作日志 */
class RegisterEditor : public QWidget {
    Q_OBJECT

public:
    explicit RegisterEditor(QWidget* parent = nullptr);
    void setConnection(IConnection* connection);    ///< 设置底层连接(I2C或SPI)
    void setDeviceAddress(int addr) { m_deviceAddress = addr; } ///< 设置I2C设备地址(7位)
    void readAddress(int address);                  ///< 读取指定地址寄存器
    void writeAddress(int address, const QByteArray& data); ///< 向指定地址写入数据
    void setReadLength(int length);                 ///< 设置默认读取长度
    int readCount() const;                          ///< 获取读操作次数
    int writeCount() const;                         ///< 获取写操作次数
    QString exportLog() const;                      ///< 导出操作日志

    quint64 totalRegisterReads() const;             ///< 累计寄存器读取次数
    quint64 totalRegisterWrites() const;            ///< 累计寄存器写入次数
    int registerCount() const { return static_cast<int>(m_accessedAddresses.size()); }
    double avgAccessTimeMs() const;                 ///< 平均寄存器访问耗时(ms)
    quint64 totalLogClears() const { return m_totalLogClears; }
    quint64 totalErrors() const { return m_totalErrors; }
    void resetStatistics();

signals:
    void registerReadComplete(int address, const QByteArray& data);
    void registerWriteComplete(int address, bool success);

private slots:
    void onReadClicked();
    void onWriteClicked();
    void onClearLogClicked();

private:
    void setupUi();
    void setupConnections();
    void appendLog(const QString& msg, bool isTx = true);
    static QString formatHex(const QByteArray& data);

    IConnection* m_connection = nullptr;
    int m_readLength = 1, m_deviceAddress = 0x00, m_readCount = 0, m_writeCount = 0;
    QSpinBox* m_addrSpin = nullptr, *m_lengthSpin = nullptr;
    QLineEdit* m_dataEdit = nullptr;
    QPushButton* m_readBtn = nullptr, *m_writeBtn = nullptr, *m_clearLogBtn = nullptr;
    QTextEdit* m_log = nullptr;
    quint64 m_totalRegisterReads = 0, m_totalRegisterWrites = 0, m_totalLogClears = 0, m_totalErrors = 0;
    QSet<int> m_accessedAddresses;
    qint64 m_totalAccessTimeUs = 0;
};

#endif // REGISTEREDITOR_H
