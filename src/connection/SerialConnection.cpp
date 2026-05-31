#include "SerialConnection.h"
#include <QDebug>

SerialConnection::SerialConnection(QObject* parent)
    : IConnection(parent)
{
    // 连接QSerialPort的信号
    connect(&m_serial, &QSerialPort::readyRead,
            this, &SerialConnection::onReadyRead);
    connect(&m_serial, &QSerialPort::errorOccurred,
            this, &SerialConnection::onError);
}

SerialConnection::~SerialConnection()
{
    close();
}

QString SerialConnection::name() const
{
    return m_portName;
}

ConnectionState SerialConnection::state() const
{
    return m_state;
}

bool SerialConnection::open()
{
    if (m_portName.isEmpty()) {
        emit errorOccurred("Port name is empty");
        return false;
    }

    m_serial.setPortName(m_portName);

    if (!m_serial.open(QIODevice::ReadWrite)) {
        m_state = ConnectionState::Error;
        emit stateChanged(m_state);
        emit errorOccurred(m_serial.errorString());
        return false;
    }

    m_state = ConnectionState::Connected;
    emit stateChanged(m_state);
    qDebug() << "Serial opened:" << m_portName
             << "baud:" << m_serial.baudRate();
    return true;
}

void SerialConnection::close()
{
    if (m_serial.isOpen()) {
        m_serial.close();
        m_state = ConnectionState::Disconnected;
        emit stateChanged(m_state);
        qDebug() << "Serial closed:" << m_portName;
    }
}

qint64 SerialConnection::write(const QByteArray& data)
{
    if (!m_serial.isOpen()) {
        return -1;
    }
    return m_serial.write(data);
}

void SerialConnection::setPortName(const QString& portName)
{
    m_portName = portName;
}

QString SerialConnection::portName() const
{
    return m_portName;
}

void SerialConnection::setBaudRate(qint32 baud)
{
    m_serial.setBaudRate(baud);
}

qint32 SerialConnection::baudRate() const
{
    return m_serial.baudRate();
}

void SerialConnection::setDataBits(QSerialPort::DataBits bits)
{
    m_serial.setDataBits(bits);
}

QSerialPort::DataBits SerialConnection::dataBits() const
{
    return m_serial.dataBits();
}

void SerialConnection::setParity(QSerialPort::Parity parity)
{
    m_serial.setParity(parity);
}

QSerialPort::Parity SerialConnection::parity() const
{
    return m_serial.parity();
}

void SerialConnection::setStopBits(QSerialPort::StopBits bits)
{
    m_serial.setStopBits(bits);
}

QSerialPort::StopBits SerialConnection::stopBits() const
{
    return m_serial.stopBits();
}

void SerialConnection::setFlowControl(QSerialPort::FlowControl control)
{
    m_serial.setFlowControl(control);
}

QSerialPort::FlowControl SerialConnection::flowControl() const
{
    return m_serial.flowControl();
}

void SerialConnection::setDtr(bool enabled)
{
    m_serial.setDataTerminalReady(enabled);
}

void SerialConnection::setRts(bool enabled)
{
    m_serial.setRequestToSend(enabled);
}

QList<QSerialPortInfo> SerialConnection::availablePorts()
{
    return QSerialPortInfo::availablePorts();
}

void SerialConnection::onReadyRead()
{
    // 从串口读取所有可用数据，转发给上层
    QByteArray data = m_serial.readAll();
    if (!data.isEmpty()) {
        emit dataReceived(data);
    }
}

void SerialConnection::onError(QSerialPort::SerialPortError error)
{
    // 忽略无错误的情况 (Qt在某些操作后会触发NoError)
    if (error == QSerialPort::NoError) {
        return;
    }

    QString errorMsg = m_serial.errorString();
    qWarning() << "Serial error:" << error << errorMsg;

    m_state = ConnectionState::Error;
    emit stateChanged(m_state);
    emit errorOccurred(errorMsg);
}
