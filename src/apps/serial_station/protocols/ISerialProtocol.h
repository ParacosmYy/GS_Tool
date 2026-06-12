#ifndef I_SERIAL_PROTOCOL_H
#define I_SERIAL_PROTOCOL_H

#include <QtCore/QByteArray>
#include <QtCore/QString>
#include <QtCore/QVariantMap>
#include <QtCore/QVector>

#include "apps/serial_station/protocols/SerialProtocolEvent.h"

namespace serial_station {

/**
 * @brief Serial Station 协议接口。
 *
 * 具体协议负责命令构建和流式解析，不依赖 UI、串口线程或文件服务。
 */
class ISerialProtocol {
public:
    virtual ~ISerialProtocol() = default;

    virtual QString name() const = 0;
    virtual QByteArray buildCommand(const QString& command, const QVariantMap& params) const = 0;
    virtual QVector<SerialProtocolEvent> feed(const QByteArray& data) = 0;
    virtual void reset() = 0;
};

} // namespace serial_station

#endif // I_SERIAL_PROTOCOL_H
