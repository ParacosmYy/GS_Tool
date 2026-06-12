#ifndef SERIAL_PROTOCOL_REGISTRY_H
#define SERIAL_PROTOCOL_REGISTRY_H

#include <QtCore/QHash>
#include <QtCore/QString>
#include <QtCore/QStringList>

#include <functional>
#include <memory>

#include "apps/serial_station/protocols/ISerialProtocol.h"

namespace serial_station {

/**
 * @brief Serial Station 协议注册表。
 *
 * 对外只暴露协议接口工厂，调用方不需要知道具体协议类路径。
 */
class SerialProtocolRegistry {
public:
    using ProtocolFactory = std::function<std::unique_ptr<ISerialProtocol>()>;

    bool registerProtocol(const QString& name, ProtocolFactory factory);
    void registerBuiltInProtocols();
    bool contains(const QString& name) const;
    QStringList protocolNames() const;
    std::unique_ptr<ISerialProtocol> create(const QString& name) const;

    bool setDefaultProtocol(const QString& name);
    QString defaultProtocol() const;
    std::unique_ptr<ISerialProtocol> createDefault() const;

private:
    static QString normalizeName(const QString& name);

    QHash<QString, ProtocolFactory> m_factories;
    QHash<QString, QString> m_displayNames;
    QString m_defaultProtocol;
};

} // namespace serial_station

#endif // SERIAL_PROTOCOL_REGISTRY_H
