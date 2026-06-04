/**
 * @file SerialConnection.cpp
 * @brief 串口连接实现 - 封装 QSerialPort 的完整串口通信逻辑
 */

#include "connection/serial_port/SerialConnection.h"
#include <QDebug>
#include <QTimer>
#include <QVariant>

/** @brief 构造串口连接，连接QSerialPort的readyRead/errorOccurred/bytesWritten信号 @param parent 父对象 */
SerialConnection::SerialConnection(QObject* parent)
    : IConnection(parent)
{
    // 连接QSerialPort的信号
    connect(&m_serial, &QSerialPort::readyRead,
            this, &SerialConnection::onReadyRead);
    connect(&m_serial, &QSerialPort::errorOccurred,
            this, &SerialConnection::onError);
    connect(&m_serial, &QSerialPort::bytesWritten,
            this, &SerialConnection::onBytesWritten);
}

/** @brief 析构函数，静默关闭串口(不发射信号，避免析构期间信号回调访问半销毁对象) */
SerialConnection::~SerialConnection()
{
    // 析构时静默关闭: 仅关闭物理端口，不发射stateChanged信号
    // 正常关闭由ConnectionManager::~ConnectionManager()通过close()完成
    if (m_serial.isOpen()) {
        m_serial.close();
        m_state = ConnectionState::Disconnected;
    }
}

/** @brief 返回端口名称 @return 当前串口端口名称字符串 */
QString SerialConnection::name() const
{
    return m_portName;
}

/** @brief 返回当前连接状态 @return ConnectionState枚举值 */
ConnectionState SerialConnection::state() const
{
    return m_state;
}

/** @brief 打开串口连接，包含端口名空检查、端口存在性检查、ReadWrite模式打开、错误翻译 @return true=打开成功, false=打开失败 */
bool SerialConnection::open()
{
    // 统计: 每次调用 open() 都计入打开次数
    m_totalOpens++;

    // 步骤1: 检查端口名
    if (m_portName.isEmpty()) {
        QString msg = tr("串口打开失败: 端口名为空，请先选择一个串口");
        m_state = ConnectionState::Error;
        emit stateChanged(m_state);
        emit errorOccurred(msg);
        return false;
    }

    // 步骤2: 检查端口是否存在于系统中
    bool portExists = false;
    const auto ports = QSerialPortInfo::availablePorts();
    for (const auto& info : ports) {
        if (info.portName() == m_portName) {
            portExists = true;
            break;
        }
    }
    if (!portExists) {
        QString msg = tr("串口打开失败: 端口 %1 不存在，设备可能已断开，请刷新端口列表")
                          .arg(m_portName);
        m_state = ConnectionState::Error;
        emit stateChanged(m_state);
        emit errorOccurred(msg);
        return false;
    }

    // 步骤3: 设置端口名并尝试打开
    m_serial.setPortName(m_portName);

    if (!m_serial.open(QIODevice::ReadWrite)) {
        // 步骤4: 打开失败，翻译错误为具体的中文诊断
        QString errorMsg = translateError(m_serial.error());
        m_state = ConnectionState::Error;
        emit stateChanged(m_state);
        emit errorOccurred(errorMsg);
        return false;
    }

    // 步骤5: 打开成功
    m_state = ConnectionState::Connected;
    resetErrorCounters();  // 每次打开串口时重置错误计数器
    emit stateChanged(m_state);
    qDebug() << "Serial opened:" << m_portName
             << "baud:" << m_serial.baudRate();
    return true;
}

/** @brief 关闭串口连接，仅在端口处于打开状态时执行关闭，关闭后状态重置为Disconnected并通知上层 */
void SerialConnection::close()
{
    if (m_serial.isOpen()) {
        // 统计: 仅在端口确实打开时计入关闭次数
        m_totalCloses++;
        m_serial.close();
        m_state = ConnectionState::Disconnected;
        emit stateChanged(m_state);
        qDebug() << "Serial closed:" << m_portName;
    }
}

// 错误处理方法(queryPlatformErrors/onError/translateError/onBytesWritten/
// sendBreak/resetErrorCounters/pinoutSignals)已拆分至 SerialConnectionError.cpp
// 配置与统计方法(setPortName/baudRate/dataBits/parity/stopBits/flowControl/
// dtr/rts/configure/availablePorts/onReadyRead/resetStats)已拆分至 SerialConnectionStats.cpp

/** @brief 写入数据到串口，串口必须处于打开状态，失败时通过errorOccurred信号通知上层 @param data 待发送的原始字节数据 @return 实际写入的字节数，-1表示串口未打开或写入失败 */
qint64 SerialConnection::write(const QByteArray& data)
{
    ++m_totalWrites;
    if (!m_serial.isOpen()) {
        emit errorOccurred(tr("发送失败: 串口 %1 未打开").arg(m_portName));
        return -1;
    }

    qint64 written = m_serial.write(data);
    if (written < 0) {
        // QSerialPort::write() 返回 -1 表示写入失败
        QString errorMsg = translateError(m_serial.error());
        emit errorOccurred(tr("串口 %1 写入失败: %2").arg(m_portName, errorMsg));
    } else {
        // 累计写入字节统计
        m_totalBytesWritten += static_cast<quint64>(written);
        // 刷新缓冲区确保数据立即发出，避免 OTA 等时序敏感场景延迟
        m_serial.flush();
    }
    return written;
}

