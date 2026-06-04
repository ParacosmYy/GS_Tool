/**
 * @file DataConverter.cpp
 * @brief 数据格式转换引擎实现
 * @author Serial Tool Team
 * @date 2026-06-02
 */

#include "utils/converter/DataConverter.h"

#include <QUrl>

/** @brief 构造函数 @param parent 父对象 */
DataConverter::DataConverter(QObject *parent)
    : QObject(parent)
{
}

/** @brief 格式转换，先解码为原始字节再编码为目标格式，同格式直接返回 @param input 输入数据 @param from 源格式 @param to 目标格式 @return 转换后的字节数据 */
QByteArray DataConverter::convert(const QByteArray &input, Format from, Format to) const
{
    if (from == to) {
        return input;
    }
    ++m_convCount;
    ++m_totalFormatSwitches;
    m_totalBytesConverted += static_cast<quint64>(input.size());
    QByteArray raw = decodeToRaw(input, from);
    if (raw.isEmpty() && !input.isEmpty()) {
        ++m_totalErrors;
    }
    return encodeFromRaw(raw, to);
}

/** @brief 自动检测数据格式(启发式)，依次尝试Binary/Hex/Octal/Base64/UrlEncode，最终回退到Ascii @param data 待检测的数据 @return 检测到的格式枚举 */
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

/** @brief 获取格式的标准名称字符串 @param format 格式枚举 @return 格式名称(如"Hex"/"ASCII"/"Base64"等) */
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

// decodeToRaw/encodeFromRaw见 DataConverterCodec.cpp

/** @brief 获取格式的人类可读中文描述 @param format 格式枚举 @return 包含格式特点和用途的中文描述 */
QString DataConverter::formatDescription(Format format)
{
    switch (format) {
    case Hex:       return tr("十六进制表示，每字节用两个字符表示，空格分隔");
    case Ascii:     return tr("ASCII纯文本，可读字符串");
    case Base64:    return tr("Base64编码，适用于二进制数据的文本传输");
    case UrlEncode: return tr("URL百分号编码，适用于网络传输中的特殊字符");
    case Binary:    return tr("二进制表示，每字节8位，空格分隔");
    case Decimal:   return tr("十进制表示，每字节0-255，空格分隔");
    case Octal:     return tr("八进制表示，每字节0-377，空格分隔");
    }
    return QString();
}

/** @brief 获取所有支持的格式列表 @return 格式枚举列表(Hex/Ascii/Base64/UrlEncode/Binary/Decimal/Octal) */
QList<DataConverter::Format> DataConverter::supportedFormats()
{
    return { Hex, Ascii, Base64, UrlEncode, Binary, Decimal, Octal };
}

/** @brief 将输入数据转换到所有其他格式(排除源格式) @param input 输入数据 @param from 源格式 @return 格式名称到转换结果的映射表 */
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

/** @brief 获取累计转换次数 @return 转换总次数 */
quint64 DataConverter::conversionCount() const
{
    return m_convCount;
}

/** @brief 获取累计转换的字节总数 @return 转换字节总数 */
quint64 DataConverter::totalBytesConverted() const
{
    return m_totalBytesConverted;
}

/** @brief 获取累计转换失败次数 @return 错误总次数 */
quint64 DataConverter::totalErrors() const
{
    return m_totalErrors;
}

/** @brief 重置所有转换统计计数器(转换次数/字节数/错误次数/格式切换归零) */
void DataConverter::resetStatistics()
{
    m_convCount = 0;
    m_totalBytesConverted = 0;
    m_totalErrors = 0;
    m_totalFormatSwitches = 0;
}
