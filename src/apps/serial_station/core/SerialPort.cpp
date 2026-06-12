#include "apps/serial_station/core/SerialPort.h"

namespace serial_station {

SerialPort::SerialPort(QObject* parent)
    : QObject(parent)
{
    connect(&m_port, &QSerialPort::readyRead, this, &SerialPort::handleReadyRead);
    connect(&m_port, &QSerialPort::errorOccurred, this, &SerialPort::handleError);
}

void SerialPort::configure(const SerialPortConfig& config)
{
    m_config = config;
}

SerialPortConfig SerialPort::config() const
{
    return m_config;
}

bool SerialPort::open()
{
    const QString validationError = m_config.validationError();
    if (!validationError.isEmpty()) {
        emit errorOccurred(validationError);
        return false;
    }

    if (m_port.isOpen()) {
        m_port.close();
    }

    m_port.setPortName(m_config.normalizedPortName());
    m_port.setBaudRate(m_config.baudRate);
    m_port.setDataBits(m_config.dataBits);
    m_port.setParity(m_config.parity);
    m_port.setStopBits(m_config.stopBits);
    m_port.setFlowControl(m_config.flowControl);

    if (!m_port.open(QIODevice::ReadWrite)) {
        emit errorOccurred(m_port.errorString());
        return false;
    }

    m_port.setDataTerminalReady(m_config.dtrEnabled);
    m_port.setRequestToSend(m_config.rtsEnabled);
    return true;
}

void SerialPort::close()
{
    if (m_port.isOpen()) {
        m_port.close();
    }
}

bool SerialPort::isOpen() const
{
    return m_port.isOpen();
}

qint64 SerialPort::write(const QByteArray& bytes)
{
    if (!m_port.isOpen() || bytes.isEmpty()) {
        return 0;
    }
    return m_port.write(bytes);
}

QString SerialPort::errorString() const
{
    return m_port.errorString();
}

void SerialPort::handleReadyRead()
{
    const QByteArray bytes = m_port.readAll();
    if (!bytes.isEmpty()) {
        emit bytesReceived(bytes);
    }
}

void SerialPort::handleError(QSerialPort::SerialPortError error)
{
    if (error != QSerialPort::NoError) {
        emit errorOccurred(m_port.errorString());
    }
}

} // namespace serial_station
