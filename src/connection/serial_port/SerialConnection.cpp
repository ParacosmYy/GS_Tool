/**
 * @file SerialConnection.cpp
 * @brief 串口连接实现 - 封装 QSerialPort 的完整串口通信逻辑
 */

#include "connection/serial_port/SerialConnection.h"
#include <QDebug>
#include <QTimer>
#include <QVariant>

/**
 * @brief 构造串口连接
 *
 * 在构造时立即连接 QSerialPort 的两个关键信号:
 *   - readyRead: 有数据到达时触发 onReadyRead() 读取并转发
 *   - errorOccurred: 发生错误时触发 onError() 翻译错误并通知上层
 *
 * @param parent 父对象
 */
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

/** @brief 返回端口名称 */
QString SerialConnection::name() const
{
    return m_portName;
}

/** @brief 返回当前连接状态 */
ConnectionState SerialConnection::state() const
{
    return m_state;
}

/**
 * @brief 打开串口连接
 *
 * 完整的打开流程:
 *   1. 检查端口名是否为空
 *   2. 检查端口是否存在于系统可用端口列表中
 *   3. 设置端口名并尝试以 ReadWrite 模式打开
 *   4. 打开失败时根据错误类型给出具体的中文诊断信息
 *   5. 打开成功后更新状态为 Connected
 *
 * @return true=打开成功, false=打开失败
 */
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

/**
 * @brief 关闭串口连接
 *
 * 仅在串口处于打开状态时执行关闭操作，避免重复关闭。
 * 关闭后将状态重置为 Disconnected 并通知上层。
 */
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

/**
 * @brief 写入数据到串口
 *
 * 前置检查: 串口必须处于打开状态。
 * 写入失败时通过 errorOccurred 信号通知上层。
 *
 * @param data 待发送的原始字节数据
 * @return 实际写入的字节数，-1表示串口未打开或写入失败
 */
qint64 SerialConnection::write(const QByteArray& data)
{
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

/** @brief 设置端口名 */
void SerialConnection::setPortName(const QString& portName)
{
    m_portName = portName;
}

/** @brief 获取当前端口名 */
QString SerialConnection::portName() const
{
    return m_portName;
}

/** @brief 设置波特率 — 检查返回值，失败时输出警告 */
void SerialConnection::setBaudRate(qint32 baud)
{
    if (!m_serial.setBaudRate(baud)) {
        qWarning() << tr("设置波特率失败, baud=%1").arg(baud);
    }
}

/** @brief 获取当前波特率 */
qint32 SerialConnection::baudRate() const
{
    return m_serial.baudRate();
}

/** @brief 设置数据位 — 检查返回值，失败时输出警告 */
void SerialConnection::setDataBits(QSerialPort::DataBits bits)
{
    if (!m_serial.setDataBits(bits)) {
        qWarning() << tr("设置数据位失败, bits=%1").arg(bits);
    }
}

/** @brief 获取当前数据位 */
QSerialPort::DataBits SerialConnection::dataBits() const
{
    return m_serial.dataBits();
}

/** @brief 设置校验模式 — 检查返回值，失败时输出警告 */
void SerialConnection::setParity(QSerialPort::Parity parity)
{
    if (!m_serial.setParity(parity)) {
        qWarning() << tr("设置校验模式失败, parity=%1").arg(parity);
    }
}

/** @brief 获取当前校验模式 */
QSerialPort::Parity SerialConnection::parity() const
{
    return m_serial.parity();
}

/** @brief 设置停止位 — 检查返回值，失败时输出警告 */
void SerialConnection::setStopBits(QSerialPort::StopBits bits)
{
    if (!m_serial.setStopBits(bits)) {
        qWarning() << tr("设置停止位失败, bits=%1").arg(bits);
    }
}

/** @brief 获取当前停止位 */
QSerialPort::StopBits SerialConnection::stopBits() const
{
    return m_serial.stopBits();
}

/** @brief 设置流控模式 — 检查返回值，失败时输出警告 */
void SerialConnection::setFlowControl(QSerialPort::FlowControl control)
{
    if (!m_serial.setFlowControl(control)) {
        qWarning() << tr("设置流控模式失败, control=%1").arg(control);
    }
}

/** @brief 获取当前流控模式 */
QSerialPort::FlowControl SerialConnection::flowControl() const
{
    return m_serial.flowControl();
}

/** @brief 设置 DTR 信号电平 */
void SerialConnection::setDtr(bool enabled)
{
    m_serial.setDataTerminalReady(enabled);
}

/** @brief 设置 RTS 信号电平 */
void SerialConnection::setRts(bool enabled)
{
    m_serial.setRequestToSend(enabled);
}

/** @brief 查询 DTR 信号当前状态 */
bool SerialConnection::isDtr() const
{
    // QSerialPort::isDataTerminalReady() 非 const，需要 const_cast
    return const_cast<QSerialPort&>(m_serial).isDataTerminalReady();
}

/** @brief 查询 RTS 信号当前状态 */
bool SerialConnection::isRts() const
{
    // QSerialPort::isRequestToSend() 非 const，需要 const_cast
    return const_cast<QSerialPort&>(m_serial).isRequestToSend();
}

/**
 * @brief 通过参数映射配置串口（工厂模式下的统一配置入口）
 *
 * 逐项检查 params 中的 key 并设置对应的串口参数。
 * 未识别的 key 会被安全忽略，不会报错。
 * 所有数值型参数都做了范围检查，超出范围的值会被跳过。
 *
 * @param params 参数映射表，支持的 key 见头文件 configure() 文档
 */
void SerialConnection::configure(const QVariantMap& params)
{
    if (params.contains("portName"))
        setPortName(params["portName"].toString());
    if (params.contains("baudRate"))
        setBaudRate(params["baudRate"].toInt());
    if (params.contains("dataBits")) {
        int db = params["dataBits"].toInt();
        QSerialPort::DataBits bits[] = {
            QSerialPort::Data5, QSerialPort::Data6,
            QSerialPort::Data7, QSerialPort::Data8
        };
        if (db >= 5 && db <= 8) setDataBits(bits[db - 5]);
    }
    if (params.contains("parity")) {
        QSerialPort::Parity p[] = {
            QSerialPort::NoParity, QSerialPort::EvenParity,
            QSerialPort::OddParity, QSerialPort::MarkParity,
            QSerialPort::SpaceParity
        };
        int idx = params["parity"].toInt();
        if (idx >= 0 && idx <= 4) setParity(p[idx]);
    }
    if (params.contains("stopBits")) {
        QSerialPort::StopBits s[] = {
            QSerialPort::OneStop, QSerialPort::OneAndHalfStop,
            QSerialPort::TwoStop
        };
        int idx = params["stopBits"].toInt();
        if (idx >= 0 && idx <= 2) setStopBits(s[idx]);
    }
    if (params.contains("flowControl")) {
        QSerialPort::FlowControl f[] = {
            QSerialPort::NoFlowControl, QSerialPort::HardwareControl,
            QSerialPort::SoftwareControl
        };
        int idx = params["flowControl"].toInt();
        if (idx >= 0 && idx <= 2) setFlowControl(f[idx]);
    }
    if (params.contains("dtr"))
        setDtr(params["dtr"].toBool());
    if (params.contains("rts"))
        setRts(params["rts"].toBool());
}

/**
 * @brief 获取系统中所有可用的串口列表
 * @return QSerialPortInfo 列表
 */
QList<QSerialPortInfo> SerialConnection::availablePorts()
{
    return QSerialPortInfo::availablePorts();
}

/**
 * @brief QSerialPort::readyRead 信号处理
 *
 * 读取串口缓冲区中的所有可用数据，并通过 IConnection::dataReceived 信号转发给上层。
 * 空数据不会触发信号，避免无意义的处理。
 */
void SerialConnection::onReadyRead()
{
    QByteArray data = m_serial.readAll();
    if (!data.isEmpty()) {
        m_totalBytesRead += static_cast<quint64>(data.size());  // 累计读取字节统计
        emit dataReceived(data);
    }
}

/**
 * @brief 重置所有操作统计计数器
 *
 * 将 totalOpens/totalCloses/totalBytesWritten/totalBytesRead/errorCount 归零。
 * 不影响 SerialErrorCounters (帧错误/校验错误等)，
 * 那些由 resetErrorCounters() 单独管理。
 */
void SerialConnection::resetStats()
{
    m_totalOpens = 0;
    m_totalCloses = 0;
    m_totalBytesWritten = 0;
    m_totalBytesRead = 0;
    m_errorCount = 0;
}
