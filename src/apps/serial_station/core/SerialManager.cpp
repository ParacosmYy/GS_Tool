#include "apps/serial_station/core/SerialManager.h"

#include "apps/serial_station/core/SerialPort.h"

namespace serial_station {

SerialManager::SerialManager(QObject* parent)
    : SerialManager(std::make_unique<SerialPort>(), parent)
{
    m_portType = SerialTransportType::Uart;
}

SerialManager::SerialManager(std::unique_ptr<ISerialPort> port, QObject* parent)
    : QObject(parent)
    , m_port(std::move(port))
{
    m_portType = SerialTransportType::Uart;
    if (!m_port) {
        m_port = std::make_unique<SerialPort>();
    } else {
        const SerialPortConfig current = m_port->config();
        if (!current.isSerial()) {
            m_portType = current.transportType;
        }
    }
    Q_ASSERT(m_port != nullptr);
    connect(m_port.get(), &ISerialPort::bytesReceived, this, &SerialManager::bytesReceived);
    connect(m_port.get(), &ISerialPort::errorOccurred, this, &SerialManager::handlePortError);
}

void SerialManager::configure(const SerialPortConfig& config)
{
    const SerialPortConfig normalizedConfig = config.normalized();
    ensurePortIfNeeded(normalizedConfig);
    m_session.setConfig(normalizedConfig);
    m_port->configure(normalizedConfig);
}

void SerialManager::ensurePortIfNeeded(const SerialPortConfig& config)
{
    if (m_portType == config.transportType && m_port) {
        return;
    }

    disconnect(m_port.get(), &ISerialPort::bytesReceived, this, &SerialManager::bytesReceived);
    disconnect(m_port.get(), &ISerialPort::errorOccurred, this, &SerialManager::handlePortError);
    if (m_port->isOpen()) {
        m_port->close();
    }

    m_port = createPortForConfig(config);
    m_portType = config.transportType;
    connect(m_port.get(), &ISerialPort::bytesReceived, this, &SerialManager::bytesReceived);
    connect(m_port.get(), &ISerialPort::errorOccurred, this, &SerialManager::handlePortError);
}

std::unique_ptr<ISerialPort> SerialManager::createPortForConfig(const SerialPortConfig& config)
{
    switch (config.transportType) {
    case SerialTransportType::TcpClient:
        return std::make_unique<TcpSerialPort>();
    case SerialTransportType::UdpClient:
        return std::make_unique<UdpSerialPort>();
    case SerialTransportType::Uart:
        return std::make_unique<SerialPort>();
    default:
        return std::make_unique<SerialPort>();
    }
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

    if (!m_port->open()) {
        m_session.markError(m_port->errorString());
        emit stateChanged(m_session.state());
        return false;
    }

    m_session.markOpen();
    emit stateChanged(m_session.state());
    return true;
}

void SerialManager::close()
{
    m_port->close();
    m_session.markClosed();
    emit stateChanged(m_session.state());
}

qint64 SerialManager::send(const QByteArray& bytes)
{
    if (bytes.isEmpty()) {
        return 0;
    }

    constexpr int kMaxWriteRetries = 3;
    qint64 totalWritten = 0;
    int retryCount = 0;
    const qint64 totalBytes = bytes.size();

    while (totalWritten < totalBytes && retryCount < kMaxWriteRetries) {
        const QByteArray remainingBytes = bytes.mid(static_cast<int>(totalWritten));
        const qint64 written = m_port->write(remainingBytes);
        if (written > 0) {
            totalWritten += written;
            retryCount = 0;
            continue;
        }

        ++retryCount;
    }

    return totalWritten;
}

void SerialManager::handlePortError(const QString& message)
{
    m_session.markError(message);
    emit errorOccurred(message);
    emit stateChanged(m_session.state());
}

} // namespace serial_station
