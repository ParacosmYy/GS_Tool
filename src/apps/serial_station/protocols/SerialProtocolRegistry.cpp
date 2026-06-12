#include "apps/serial_station/protocols/SerialProtocolRegistry.h"

#include <QtCore/QLocale>

#include "apps/serial_station/SerialStationConstants.h"
#include "apps/serial_station/protocols/ascii_text/AsciiTextProtocol.h"
#include "apps/serial_station/protocols/custom_md/CustomMdProtocol.h"
#include "apps/serial_station/protocols/modbus_rtu/ModbusRtuProtocol.h"

namespace serial_station {

bool SerialProtocolRegistry::registerProtocol(const QString& name, ProtocolFactory factory)
{
    const QString key = normalizeName(name);
    if (key.isEmpty() || !factory || m_factories.contains(key)) {
        return false;
    }

    m_factories.insert(key, std::move(factory));
    m_displayNames.insert(key, name.trimmed());
    if (m_defaultProtocol.isEmpty()) {
        m_defaultProtocol = key;
    }
    return true;
}

void SerialProtocolRegistry::registerBuiltInProtocols()
{
    registerProtocol(serialStationConstants::kDefaultProtocolName, [] {
        return std::make_unique<AsciiTextProtocol>();
    });
    registerProtocol(QStringLiteral("modbus_rtu"), [] {
        return std::make_unique<ModbusRtuProtocol>();
    });
    registerProtocol(QStringLiteral("custom_md"), [] {
        return std::make_unique<CustomMdProtocol>();
    });
    setDefaultProtocol(serialStationConstants::kDefaultProtocolName);
}

bool SerialProtocolRegistry::contains(const QString& name) const
{
    return m_factories.contains(normalizeName(name));
}

QStringList SerialProtocolRegistry::protocolNames() const
{
    QStringList names = m_displayNames.values();
    names.sort(Qt::CaseInsensitive);
    return names;
}

std::unique_ptr<ISerialProtocol> SerialProtocolRegistry::create(const QString& name) const
{
    const QString key = normalizeName(name);
    if (!m_factories.contains(key)) {
        return nullptr;
    }
    return m_factories.value(key)();
}

bool SerialProtocolRegistry::setDefaultProtocol(const QString& name)
{
    const QString key = normalizeName(name);
    if (!m_factories.contains(key)) {
        return false;
    }
    m_defaultProtocol = key;
    return true;
}

QString SerialProtocolRegistry::defaultProtocol() const
{
    return m_displayNames.value(m_defaultProtocol);
}

std::unique_ptr<ISerialProtocol> SerialProtocolRegistry::createDefault() const
{
    return create(m_defaultProtocol);
}

QString SerialProtocolRegistry::normalizeName(const QString& name)
{
    return name.trimmed().toLower();
}

} // namespace serial_station
