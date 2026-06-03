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
    ++m_convCount;
    m_totalBytesConverted += static_cast<quint64>(input.size());
    QByteArray raw = decodeToRaw(input, from);
    if (raw.isEmpty() && !input.isEmpty()) {
        ++m_totalErrors;
    }
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

/**
 * @brief 获取格式的人类可读描述
 */
QString DataConverter::formatDescription(Format format)
{
    switch (format) {
    case Hex:       return QStringLiteral("十六进制表示，每字节用两个字符表示，空格分隔");
    case Ascii:     return QStringLiteral("ASCII纯文本，可读字符串");
    case Base64:    return QStringLiteral("Base64编码，适用于二进制数据的文本传输");
    case UrlEncode: return QStringLiteral("URL百分号编码，适用于网络传输中的特殊字符");
    case Binary:    return QStringLiteral("二进制表示，每字节8位，空格分隔");
    case Decimal:   return QStringLiteral("十进制表示，每字节0-255，空格分隔");
    case Octal:     return QStringLiteral("八进制表示，每字节0-377，空格分隔");
    }
    return QString();
}

/**
 * @brief 获取所有支持的格式列表
 */
QList<DataConverter::Format> DataConverter::supportedFormats()
{
    return { Hex, Ascii, Base64, UrlEncode, Binary, Decimal, Octal };
}

/**
 * @brief 将输入数据转换到所有其他格式
 */
QMap<QString, QByteArray> DataConverter::convertToAll(const QByteArray& input, Format from) const
{
    QMap<QString, QByteArray> results;
    const QList<Format> formats = supportedFormats();
    for (Format fmt : formats) {
        if (fmt == from) {
            continue;
        }
        results[formatName(fmt)] = convert(input, from, fmt);
    }
    return results;
}

/**
 * @brief 获取累计转换次数
 */
quint64 DataConverter::conversionCount() const
{
    return m_convCount;
}

/**
 * @brief 获取累计转换的字节总数
 */
quint64 DataConverter::totalBytesConverted() const
{
    return m_totalBytesConverted;
}

/**
 * @brief 获取累计转换失败次数
 */
quint64 DataConverter::totalErrors() const
{
    return m_totalErrors;
}

/**
 * @brief 重置所有转换统计计数器(转换次数/字节数/错误次数)
 */
void DataConverter::resetStatistics()
{
    m_convCount = 0;
    m_totalBytesConverted = 0;
    m_totalErrors = 0;
}
