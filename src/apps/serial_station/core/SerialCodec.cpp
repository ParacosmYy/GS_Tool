#include "apps/serial_station/core/SerialCodec.h"

#include <QtCore/QCoreApplication>
#include <QtCore/QVariantMap>

#include "utils/crypto/HexConverter.h"

namespace serial_station {

namespace {

QString codecText(const char* sourceText)
{
    return QCoreApplication::translate("SerialCodec", sourceText);
}

QString compactHexText(const QString& text)
{
    QString compact = text.trimmed();
    compact.remove(QLatin1Char(' '));
    compact.remove(QLatin1Char(','));
    compact.remove(QLatin1Char('\n'));
    compact.remove(QLatin1Char('\r'));
    compact.remove(QLatin1Char('\t'));
    compact.remove(QStringLiteral("0x"), Qt::CaseInsensitive);
    return compact;
}

bool containsOnlyHexDigits(const QString& text)
{
    for (const QChar ch : text) {
        if (!ch.isDigit() &&
            (ch.toLower() < QLatin1Char('a') || ch.toLower() > QLatin1Char('f'))) {
            return false;
        }
    }

    return true;
}

} // namespace

QString SerialCodec::normalizeMode(const QString& mode) const
{
    return mode.trimmed().toLower();
}

SerialCodec::EncodeResult SerialCodec::encode(const QString& command,
                                              const QString& mode,
                                              const ISerialProtocol* protocol) const
{
    const QString normalizedMode = normalizeMode(mode);
    const QString trimmedCommand = command.trimmed();
    if (trimmedCommand.isEmpty()) {
        return makeError(normalizedMode, codecText("发送内容为空"));
    }

    if (normalizedMode == QStringLiteral("ascii")) {
        return encodeAscii(trimmedCommand, normalizedMode);
    }

    if (normalizedMode == QStringLiteral("hex")) {
        return encodeHex(trimmedCommand, normalizedMode);
    }

    if (normalizedMode == QStringLiteral("protocol")) {
        return encodeProtocol(trimmedCommand, normalizedMode, protocol);
    }

    return makeError(normalizedMode,
                     codecText("发送模式暂不支持: %1").arg(mode.trimmed()));
}

SerialCodec::EncodeResult SerialCodec::encodeAscii(const QString& command,
                                                   const QString& mode) const
{
    EncodeResult result;
    result.ok = true;
    result.normalizedMode = mode;
    result.frame = command.toUtf8();
    return result;
}

SerialCodec::EncodeResult SerialCodec::encodeHex(const QString& command,
                                                 const QString& mode) const
{
    const QString compact = compactHexText(command);
    if (compact.isEmpty()) {
        return makeError(mode, codecText("HEX 内容为空"));
    }

    if (compact.length() % 2 != 0) {
        return makeError(mode, codecText("HEX 字符数量必须为偶数"));
    }

    if (!containsOnlyHexDigits(compact)) {
        return makeError(mode, codecText("HEX 内容包含非法字符"));
    }

    const QByteArray frame = HexConverter::fromHexString(compact);
    if (frame.isEmpty()) {
        return makeError(mode, codecText("HEX 内容未解析出字节"));
    }

    EncodeResult result;
    result.ok = true;
    result.normalizedMode = mode;
    result.frame = frame;
    return result;
}

SerialCodec::EncodeResult SerialCodec::encodeProtocol(const QString& command,
                                                      const QString& mode,
                                                      const ISerialProtocol* protocol) const
{
    if (!protocol) {
        return makeError(mode, codecText("默认协议不可用"));
    }

    QVariantMap params;
    params.insert(QStringLiteral("text"), command);
    params.insert(QStringLiteral("appendNewline"), false);

    const QByteArray frame = protocol->buildCommand(command, params);
    if (frame.isEmpty()) {
        return makeError(mode, codecText("协议构建出的发送帧为空"));
    }

    EncodeResult result;
    result.ok = true;
    result.normalizedMode = mode;
    result.frame = frame;
    return result;
}

SerialCodec::EncodeResult SerialCodec::makeError(const QString& mode,
                                                 const QString& message) const
{
    EncodeResult result;
    result.normalizedMode = mode;
    result.errorMessage = message;
    return result;
}

} // namespace serial_station
