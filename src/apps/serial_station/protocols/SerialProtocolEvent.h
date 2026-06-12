#ifndef SERIAL_PROTOCOL_EVENT_H
#define SERIAL_PROTOCOL_EVENT_H

#include <QtCore/QByteArray>
#include <QtCore/QString>
#include <QtCore/QVariantMap>

namespace serial_station {

/**
 * @brief 协议解析输出事件。
 *
 * 协议层只描述事实，不包含 UI 文案，也不直接更新控件。
 */
struct SerialProtocolEvent {
    QString type;
    QString protocolName;
    QVariantMap payload;
    QByteArray raw;
};

} // namespace serial_station

#endif // SERIAL_PROTOCOL_EVENT_H
