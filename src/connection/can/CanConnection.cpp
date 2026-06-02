/**
 * @file CanConnection.cpp
 * @brief CAN/CAN-FD连接实现 — 基于串口CAN适配器(LAWICEL/SLCAN协议)
 */

#include "connection/can/CanConnection.h"
#include "connection/can/CanFrameParser.h"
#include <QSerialPortInfo>

CanConnection::CanConnection(QObject* parent)
    : IConnection(parent)
{
}

CanConnection::~CanConnection()
{
    close();
}

ConnectionType CanConnection::type() const
{
    return ConnectionType::Serial;
}

QString CanConnection::name() const
{
    return m_adapterName.isEmpty() ? tr("未配置") : m_adapterName;
}

ConnectionState CanConnection::state() const
{
    return m_state;
}

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

IConnection* CanConnection::serialPort() const
{
    return m_serialPort;
}

bool CanConnection::open()
{
    if (!m_serialPort) {
        emit errorOccurred(tr("未设置底层串口连接"));
        return false;
    }
    m_state = ConnectionState::Connecting;
    emit stateChanged(m_state);

    /* 打开串口 */
    if (!m_serialPort->open()) {
        m_state = ConnectionState::Error;
        emit stateChanged(m_state);
        emit errorOccurred(tr("串口打开失败"));
        return false;
    }

    /* LAWICEL: 关闭CAN → 设置波特率 → 打开CAN */
    sendCommand("C");   // 关闭
    sendCommand("S8");  // 默认500k，后面按m_bitrate发

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
        sendCommand("#" + QByteArray(1, static_cast<char>(0x11)));  // 启用FD
    }

    sendCommand("O");   // 打开CAN通道

    m_state = ConnectionState::Connected;
    emit stateChanged(m_state);
    return true;
}

void CanConnection::close()
{
    if (m_state == ConnectionState::Disconnected) return;
    if (m_serialPort && m_serialPort->state() == ConnectionState::Connected) {
        sendCommand("C");  // LAWICEL关闭CAN
    }
    m_state = ConnectionState::Disconnected;
    emit stateChanged(m_state);
}

qint64 CanConnection::write(const QByteArray& data)
{
    if (!m_serialPort || m_state != ConnectionState::Connected) return -1;
    return m_serialPort->write(data);
}

void CanConnection::configure(const QVariantMap& params)
{
    if (params.contains("bitrate"))  m_bitrate = params.value("bitrate").toInt();
    if (params.contains("canFd"))    m_canFdEnabled = params.value("canFd").toBool();
    if (params.contains("adapter"))  m_adapterName = params.value("adapter").toString();
}

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

void CanConnection::setCanFdEnabled(bool enabled)
{
    m_canFdEnabled = enabled;
}

bool CanConnection::sendFrame(int id, const QByteArray& data, bool extended)
{
    if (m_state != ConnectionState::Connected) return false;

    CanFrame frame;
    frame.id = static_cast<quint32>(id);
    frame.data = data;
    frame.extended = extended;
    frame.rtr = false;
    frame.fd = m_canFdEnabled;
    frame.dlc = static_cast<quint8>(data.size());

    CanFrameParser parser;
    QByteArray raw = parser.buildFrame(frame);
    if (raw.isEmpty()) return false;

    raw.append('\r');  // LAWICEL命令以\r结尾
    return m_serialPort && m_serialPort->write(raw) > 0;
}

qint64 CanConnection::sendCommand(const QString& cmd)
{
    if (!m_serialPort) return -1;
    QByteArray payload = cmd.toUtf8() + '\r';
    return m_serialPort->write(payload);
}

void CanConnection::onSerialDataReceived(const QByteArray& data)
{
    m_rxBuffer.append(data);
    parseBuffer();
}

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
            continue;
        }

        /* 尝试解析为CAN帧 */
        CanFrameParser parser;
        CanFrame frame = parser.parseFrame(line);
        if (frame.dlc > 0 || frame.rtr) {
            emit frameReceived(static_cast<int>(frame.id),
                               frame.data, frame.extended, frame.rtr);
            emit dataReceived(line);  // 也向上层转发原始数据
        }
    }
}
