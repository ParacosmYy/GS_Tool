/**
 * @file DataConverter.cpp
 * @brief 数据格式转换引擎实现
 * @author Serial Tool Team
 * @date 2026-06-02
 */

#include "utils/converter/DataConverter.h"

#include <QUrl>

/**
 * @brief 构造函数
 */
DataConverter::DataConverter(QObject *parent)
    : QObject(parent)
{
}

/**
 * @brief 格式转换：先解码为原始字节，再编码为目标格式
 */
QByteArray DataConverter::convert(const QByteArray &input, Format from, Format to) const
{
    if (from == to) {
        return input;
    }
    QByteArray raw = decodeToRaw(input, from);
    return encodeFromRaw(raw, to);
}

/**
 * @brief 自动检测数据格式（启发式）
 */
DataConverter::Format DataConverter::detectFormat(const QByteArray &data) const
{
    if (data.isEmpty()) {
        return Ascii;
    }

    // 检测二进制
    bool allBinary = true;
    for (char c : data) {
        if (c != '0' && c != '1' && c != ' ') {
            allBinary = false;
            break;
        }
    }
    if (allBinary && data.contains(' ') && data.size() > 3) {
        return Binary;
    }

    // 检测十六进制
    bool allHex = true;
    for (char c : data) {
        if (!QChar::isLetterOrNumber(c) && c != ' ') {
            allHex = false;
            break;
        }
    }
    QString hexStr = QString::fromUtf8(data).simplified();
    hexStr.remove(' ');
    if (allHex && (hexStr.length() % 2 == 0)) {
        bool ok;
        hexStr.toULongLong(&ok, 16);
        if (ok) {
            return Hex;
        }
    }

    // 检测八进制
    bool allOctal = true;
    for (char c : data) {
        if ((c < '0' || c > '7') && c != ' ') {
            allOctal = false;
            break;
        }
    }
    if (allOctal) {
        return Octal;
    }

    // 检测 Base64
    QByteArray decoded = QByteArray::fromBase64(data);
    if (!decoded.isEmpty() && decoded.toBase64() == data) {
        return Base64;
    }

    // 检测 URL 编码
    if (data.contains('%') && data.contains(QByteArray("\\x"))) {
        return UrlEncode;
    }

    return Ascii;
}

/**
 * @brief 获取格式名称
 */
QString DataConverter::formatName(Format format)
{
    switch (format) {
    case Hex:       return QStringLiteral("Hex");
    case Ascii:     return QStringLiteral("ASCII");
    case Base64:    return QStringLiteral("Base64");
    case UrlEncode: return QStringLiteral("URL Encode");
    case Binary:    return QStringLiteral("Binary");
    case Decimal:   return QStringLiteral("Decimal");
    case Octal:     return QStringLiteral("Octal");
    }
    return QStringLiteral("Unknown");
}

/**
 * @brief 从指定格式解码为原始字节
 */
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

/**
 * @brief 将原始字节编码为指定格式
 */
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
