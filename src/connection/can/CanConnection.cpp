/**
 * @file CanConnection.cpp
 * @brief CAN/CAN-FD连接实现 — 基于串口CAN适配器(LAWICEL/SLCAN协议)
 */

#include "connection/can/CanConnection.h"
#include "connection/can/CanFrameParser.h"
#include <QSerialPortInfo>

/** @brief 构造函数，初始化CAN连接基类 @param parent 父对象指针 */
CanConnection::CanConnection(QObject* parent)
    : IConnection(parent)
{
}

/** @brief 析构函数，关闭连接释放资源 */
CanConnection::~CanConnection()
{
    close();
}

/** @brief 获取连接类型 @return 固定返回ConnectionType::Can */
ConnectionType CanConnection::type() const
{
    return ConnectionType::Can;
}

/** @brief 获取连接显示名称 @return 适配器名称，未配置时返回"未配置" */
QString CanConnection::name() const
{
    return m_adapterName.isEmpty() ? tr("未配置") : m_adapterName;
}

/** @brief 获取当前连接状态 @return 当前连接状态枚举值 */
ConnectionState CanConnection::state() const
{
    return m_state;
}

/** @brief 设置底层串口连接，用于CAN适配器通信 @param serialPort 串口连接实例指针 */
void CanConnection::setSerialPort(IConnection* serialPort)
{
    if (m_serialPort) {
        disconnect(m_serialPort, &IConnection::dataReceived,
                   this, &CanConnection::onSerialDataReceived);
    }
    m_serialPort = serialPort;
    if (m_serialPort) {
        connect(m_serialPort, &IConnection::dataReceived,
                this, &CanConnection::onSerialDataReceived);
    }
}

/** @brief 获取当前串口连接实例 @return 底层串口连接指针 */
IConnection* CanConnection::serialPort() const
{
    return m_serialPort;
}

/** @brief 打开CAN连接，执行LAWICEL初始化序列(关闭→设置波特率→打开) @return 成功返回true */
bool CanConnection::open()
{
    if (!m_serialPort) {
        emit errorOccurred(tr("未设置底层串口连接"));
        ++m_errorCount;
        return false;
    }
    m_state = ConnectionState::Connecting;
    emit stateChanged(m_state);

    /* 打开串口 */
    if (!m_serialPort->open()) {
        m_state = ConnectionState::Error;
        emit stateChanged(m_state);
        emit errorOccurred(tr("串口打开失败"));
        ++m_errorCount;
        return false;
    }

    /* LAWICEL: 关闭CAN → 设置波特率 → 打开CAN */
    sendCommand("C");   // 关闭

    /* 根据波特率发送S命令 */
    QString bitrateCmd;
    const int kBaud1000 = 1000000;
    if (m_bitrate >= kBaud1000)      bitrateCmd = "S8";
    else if (m_bitrate >= 800000)    bitrateCmd = "S7";
    else if (m_bitrate >= 500000)    bitrateCmd = "S6";
    else if (m_bitrate >= 250000)    bitrateCmd = "S5";
    else if (m_bitrate >= 125000)    bitrateCmd = "S4";
    else if (m_bitrate >= 100000)    bitrateCmd = "S3";
    else if (m_bitrate >= 50000)     bitrateCmd = "S2";
    else                              bitrateCmd = "S1";
    sendCommand(bitrateCmd);

    /* CAN-FD模式 */
    if (m_canFdEnabled) {
        sendCommand(QString("#") + QChar(0x11));  // 启用FD
    }

    sendCommand("O");   // 打开CAN通道

    m_state = ConnectionState::Connected;
    emit stateChanged(m_state);
    return true;
}

/** @brief 关闭CAN连接，发送LAWICEL关闭命令并断开 */
void CanConnection::close()
{
    if (m_state == ConnectionState::Disconnected) return;
    if (m_serialPort && m_serialPort->state() == ConnectionState::Connected) {
        sendCommand("C");  // LAWICEL关闭CAN
    }
    m_state = ConnectionState::Disconnected;
    emit stateChanged(m_state);
}

/** @brief 向底层串口写入原始数据 @param data 待发送的字节数据 @return 实际写入字节数，失败返回-1 */
qint64 CanConnection::write(const QByteArray& data)
{
    if (!m_serialPort || m_state != ConnectionState::Connected) {
        ++m_errorCount;
        return -1;
    }

    qint64 written = m_serialPort->write(data);

    if (written > 0) {
        ++m_totalFramesSent;
        m_totalBytesSent += static_cast<quint64>(written);
    } else {
        ++m_errorCount;
    }
    return written;
}

/** @brief 通过参数映射配置CAN连接属性(波特率/FD模式/适配器名) @param params 配置参数键值对 */
void CanConnection::configure(const QVariantMap& params)
{
    if (params.contains("bitrate"))  m_bitrate = params.value("bitrate").toInt();
    if (params.contains("canFd"))    m_canFdEnabled = params.value("canFd").toBool();
    if (params.contains("adapter"))  m_adapterName = params.value("adapter").toString();
}

/** @brief 设置CAN总线波特率，已连接时动态切换 @param bitrate 波特率值(bps) */
void CanConnection::setBitrate(int bitrate)
{
    m_bitrate = bitrate;
    /* 如果已连接，动态切换波特率 */
    if (m_state == ConnectionState::Connected && m_serialPort) {
        sendCommand("C");
        QString cmd;
        if (bitrate >= 1000000)      cmd = "S8";
        else if (bitrate >= 500000)  cmd = "S6";
        else if (bitrate >= 250000)  cmd = "S5";
        else if (bitrate >= 125000)  cmd = "S4";
        else                         cmd = "S1";
        sendCommand(cmd);
        sendCommand("O");
    }
}

/** @brief 设置CAN-FD模式启用状态 @param enabled true启用CAN-FD */
void CanConnection::setCanFdEnabled(bool enabled)
{
    m_canFdEnabled = enabled;
}

/** @brief 发送CAN帧到总线 @param id 帧ID @param data 帧数据 @param extended 是否使用扩展帧格式 @return 发送成功返回true */
bool CanConnection::sendFrame(int id, const QByteArray& data, bool extended)
{
    if (m_state != ConnectionState::Connected) {
        ++m_errorCount;
        return false;
    }

    CanFrame frame;
    frame.id = static_cast<quint32>(id);
    frame.data = data;
    frame.extended = extended;
    frame.rtr = false;
    frame.fd = m_canFdEnabled;
    frame.dlc = static_cast<quint8>(data.size());

    CanFrameParser parser;
    QByteArray raw = parser.buildFrame(frame);
    if (raw.isEmpty()) {
        ++m_errorCount;
        return false;
    }

    raw.append('\r');  // LAWICEL命令以\r结尾
    qint64 written = m_serialPort ? m_serialPort->write(raw) : 0;

    if (written > 0) {
        ++m_totalFramesSent;
        m_totalBytesSent += static_cast<quint64>(written);
    } else {
        ++m_errorCount;
    }
    return written > 0;
}

/** @brief 向适配器发送LAWICEL原始命令 @param cmd 命令字符串(不含结尾\r) @return 实际写入字节数，失败返回-1 */
qint64 CanConnection::sendCommand(const QString& cmd)
{
    if (!m_serialPort) return -1;
    QByteArray payload = cmd.toUtf8() + '\r';
    return m_serialPort->write(payload);
}

/** @brief 串口数据接收槽函数，追加到接收缓冲区并触发解析 @param data 从串口接收到的原始字节 */
void CanConnection::onSerialDataReceived(const QByteArray& data)
{
    m_rxBuffer.append(data);
    parseBuffer();
}

/** @brief 解析接收缓冲区中的LAWICEL帧，提取有效CAN帧并发射信号 */
void CanConnection::parseBuffer()
{
    /* LAWICEL帧以\r分隔，z表示ACK，\a表示错误 */
    while (m_rxBuffer.contains('\r')) {
        int idx = m_rxBuffer.indexOf('\r');
        QByteArray line = m_rxBuffer.left(idx).trimmed();
        m_rxBuffer.remove(0, idx + 1);

        if (line.isEmpty() || line == "z") continue;  // ACK或空行
        if (line.startsWith('\a')) {
            emit errorOccurred(tr("CAN适配器报告错误"));
            ++m_errorCount;
            continue;
        }

        /* 尝试解析为CAN帧 */
        CanFrameParser parser;
        CanFrame frame = parser.parseFrame(line);
        if (frame.dlc > 0 || frame.rtr) {
            /// 更新统计: 接收到有效CAN帧
            ++m_totalFramesReceived;
            m_totalBytesReceived += static_cast<quint64>(frame.data.size());

            emit frameReceived(static_cast<int>(frame.id),
                               frame.data, frame.extended, frame.rtr);
            emit dataReceived(line);  // 也向上层转发原始数据
        }
    }
}

/** @brief 重置所有统计计数器 */
void CanConnection::resetStats()
{
    m_totalFramesSent = 0;
    m_totalFramesReceived = 0;
    m_totalBytesSent = 0;
    m_totalBytesReceived = 0;
    m_errorCount = 0;
}
