#include "apps/serial_station/core/SerialSession.h"

namespace serial_station {

void SerialSession::setConfig(const SerialPortConfig& config)
{
    m_config = config;
}

SerialPortConfig SerialSession::config() const
{
    return m_config;
}

void SerialSession::markOpening()
{
    m_state = SerialSessionState::Opening;
    m_errorString.clear();
}

void SerialSession::markOpen()
{
    m_state = SerialSessionState::Open;
    m_errorString.clear();
}

void SerialSession::markClosed()
{
    m_state = SerialSessionState::Closed;
    m_errorString.clear();
}

void SerialSession::markError(const QString& message)
{
    m_state = SerialSessionState::Error;
    m_errorString = message;
}

SerialSessionState SerialSession::state() const
{
    return m_state;
}

QString SerialSession::errorString() const
{
    return m_errorString;
}

bool SerialSession::isOpen() const
{
    return m_state == SerialSessionState::Open;
}

} // namespace serial_station
