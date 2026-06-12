#ifndef SERIAL_CODEC_H
#define SERIAL_CODEC_H

#include <QtCore/QByteArray>
#include <QtCore/QString>

#include "apps/serial_station/protocols/ISerialProtocol.h"

namespace serial_station {

/**
 * @brief Serial Station 发送帧编码器。
 */
class SerialCodec {
public:
    /**
     * @brief 发送帧构建结果。
     */
    struct EncodeResult {
        bool ok = false;
        QString normalizedMode;
        QByteArray frame;
        QString errorMessage;
    };

    /**
     * @brief 归一化发送模式。
     * @param mode UI 提供的模式文本
     * @return 小写并去除首尾空白后的模式
     */
    QString normalizeMode(const QString& mode) const;

    /**
     * @brief 根据发送模式构建待写入串口的字节帧。
     * @param command 命令文本或 HEX 文本
     * @param mode 发送模式
     * @param protocol 协议模式使用的协议对象，可为空
     * @return 编码结果
     */
    EncodeResult encode(const QString& command,
                        const QString& mode,
                        const ISerialProtocol* protocol) const;

private:
    EncodeResult encodeAscii(const QString& command, const QString& mode) const;
    EncodeResult encodeHex(const QString& command, const QString& mode) const;
    EncodeResult encodeProtocol(const QString& command,
                                const QString& mode,
                                const ISerialProtocol* protocol) const;
    EncodeResult makeError(const QString& mode, const QString& message) const;
};

} // namespace serial_station

#endif // SERIAL_CODEC_H
