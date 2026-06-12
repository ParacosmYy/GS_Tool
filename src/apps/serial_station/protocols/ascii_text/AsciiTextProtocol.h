#ifndef ASCII_TEXT_PROTOCOL_H
#define ASCII_TEXT_PROTOCOL_H

#include <QtCore/QByteArray>
#include <QtCore/QString>
#include <QtCore/QVector>

#include "apps/serial_station/protocols/ISerialProtocol.h"

namespace serial_station {

/**
 * @brief ASCII/UTF-8 文本协议。
 *
 * 发送侧把 text 参数编码为 UTF-8；接收侧按换行切分 frame 事件。
 */
class AsciiTextProtocol final : public ISerialProtocol {
public:
    QString name() const override;
    QByteArray buildCommand(const QString& command, const QVariantMap& params) const override;
    QVector<SerialProtocolEvent> feed(const QByteArray& data) override;
    void reset() override;

private:
    QByteArray m_buffer;
};

} // namespace serial_station

#endif // ASCII_TEXT_PROTOCOL_H
