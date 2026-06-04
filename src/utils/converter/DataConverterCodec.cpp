/**
 * @file DataConverterCodec.cpp
 * @brief 数据格式转换引擎 - 编解码方法实现
 *
 * 从 DataConverter.cpp 拆分而来，包含各格式的解码(decodeToRaw)
 * 和编码(encodeFromRaw)核心转换逻辑。
 */

#include "utils/converter/DataConverter.h"
#include <QUrl>

/** @brief 从指定格式解码为原始字节 @param input 输入数据 @param from 源格式 @return 解码后的原始字节数据 */
QByteArray DataConverter::decodeToRaw(const QByteArray &input, Format from) const
{
    switch (from) {
    case Hex: {
        QString hex = QString::fromUtf8(input).simplified();
        hex.remove(' ');
        return QByteArray::fromHex(hex.toUtf8());
    }
    case Ascii:
        return input;
    case Base64:
        return QByteArray::fromBase64(input);
    case UrlEncode:
        return QUrl::fromPercentEncoding(input).toUtf8();
    case Binary: {
        QByteArray result;
        QString text = QString::fromUtf8(input).simplified();
        QStringList parts = text.split(' ', Qt::SkipEmptyParts);
        for (const auto &part : parts) {
            bool ok;
            quint8 byte = static_cast<quint8>(part.toUInt(&ok, 2));
            if (ok) {
                result.append(byte);
            }
        }
        return result;
    }
    case Decimal: {
        QByteArray result;
        QString text = QString::fromUtf8(input).simplified();
        QStringList parts = text.split(' ', Qt::SkipEmptyParts);
        for (const auto &part : parts) {
            bool ok;
            quint64 val = part.toULongLong(&ok, 10);
            if (ok) {
                result.append(static_cast<char>(val & 0xFF));
            }
        }
        return result;
    }
    case Octal: {
        QByteArray result;
        QString text = QString::fromUtf8(input).simplified();
        QStringList parts = text.split(' ', Qt::SkipEmptyParts);
        for (const auto &part : parts) {
            bool ok;
            quint64 val = part.toULongLong(&ok, 8);
            if (ok) {
                result.append(static_cast<char>(val & 0xFF));
            }
        }
        return result;
    }
    }
    return input;
}

/** @brief 将原始字节编码为指定格式 @param raw 原始字节数据 @param to 目标格式 @return 编码后的字节数据 */
QByteArray DataConverter::encodeFromRaw(const QByteArray &raw, Format to) const
{
    switch (to) {
    case Hex:
        return raw.toHex(' ').toUpper();
    case Ascii:
        return raw;
    case Base64:
        return raw.toBase64();
    case UrlEncode:
        return QUrl::toPercentEncoding(QString::fromUtf8(raw));
    case Binary: {
        QStringList parts;
        for (char byte : raw) {
            parts.append(QString::number(static_cast<quint8>(byte), 2).rightJustified(8, '0'));
        }
        return parts.join(' ').toUtf8();
    }
    case Decimal: {
        QStringList parts;
        for (char byte : raw) {
            parts.append(QString::number(static_cast<quint8>(byte), 10));
        }
        return parts.join(' ').toUtf8();
    }
    case Octal: {
        QStringList parts;
        for (char byte : raw) {
            parts.append(QString::number(static_cast<quint8>(byte), 8));
        }
        return parts.join(' ').toUtf8();
    }
    }
    return raw;
}
