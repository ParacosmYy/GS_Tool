#include "SerialConnection.h"
#include <QDebug>
#include <QVariant>

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

// 通过参数映射配置串口（工厂模式下的统一配置入口）
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
