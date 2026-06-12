#include "apps/serial_station/core/SerialManager.h"

namespace serial_station {

SerialManager::SerialManager(QObject* parent)
    : QObject(parent)
    , m_port(this)
{
    connect(&m_port, &SerialPort::bytesReceived, this, &SerialManager::bytesReceived);
    connect(&m_port, &SerialPort::errorOccurred, this, &SerialManager::handlePortError);
}

void SerialManager::configure(const SerialPortConfig& config)
{
    const SerialPortConfig normalizedConfig = config.normalized();
    m_session.setConfig(normalizedConfig);
    m_port.configure(normalizedConfig);
}

void SerialManager::rejectConfiguration(const SerialPortConfig& config, const QString& message)
{
    configure(config);
    m_session.markError(message);
    emit errorOccurred(message);
    emit stateChanged(m_session.state());
}

SerialSession SerialManager::session() const
{
    return m_session;
}

bool SerialManager::open()
{
    m_session.markOpening();
    emit stateChanged(m_session.state());

    if (!m_port.open()) {
        m_session.markError(m_port.errorString());
        emit stateChanged(m_session.state());
        return false;
    }

    m_session.markOpen();
    emit stateChanged(m_session.state());
    return true;
}

void SerialManager::close()
{
    m_port.close();
    m_session.markClosed();
    emit stateChanged(m_session.state());
}

qint64 SerialManager::send(const QByteArray& bytes)
{
    return m_port.write(bytes);
}

void SerialManager::handlePortError(const QString& message)
{
    m_session.markError(message);
    emit errorOccurred(message);
    emit stateChanged(m_session.state());
}

} // namespace serial_station
