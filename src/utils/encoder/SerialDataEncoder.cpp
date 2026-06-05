/**
 * @file SerialDataEncoder.cpp
 * @brief 串口数据编解码引擎实现
 * @author Serial Tool Team
 * @date 2026-06-05
 *
 * 实现 Hex/Base64/Base32/Base85/ASCII/Binary/URL/Quoted-Printable
 * 八种编码格式的编码/解码、自动检测和有效性校验。
 * 统计查询和重置方法见 SerialDataEncoderStats.cpp。
 */

#include "utils/encoder/SerialDataEncoder.h"

#include <QByteArray>
#include <QString>

/** @brief 构造串口数据编解码引擎，初始化统计 @param parent 父对象 */
SerialDataEncoder::SerialDataEncoder(QObject *parent)
    : QObject(parent)
{
}

// ── 核心编解码 API ──

/** @brief 编码原始字节数据为指定格式 @param data 原始字节 @param encoding 目标编码 @return 编码结果 */
QByteArray SerialDataEncoder::encode(const QByteArray &data, Encoding encoding)
{
    if (data.isEmpty()) return QByteArray();
    QByteArray result;
    try {
        switch (encoding) {
        case Encoding::Hex:              result = encodeHex(data); break;
        case Encoding::Base64:           result = data.toBase64(); break;
        case Encoding::Ascii:            result = data; break;
        case Encoding::Binary:           result = encodeBinary(data); break;
        case Encoding::UrlEncode:        result = encodeUrl(data); break;
        case Encoding::QuotedPrintable:  result = encodeQuotedPrintable(data); break;
        case Encoding::Base32:           result = encodeBase32(data); break;
        case Encoding::Base85:           result = encodeBase85(data); break;
        }
    } catch (...) {
        ++m_stats.encodeErrors;
        emit error(tr("编码失败: 内部异常"), encoding);
        return QByteArray();
    }
    ++m_stats.totalEncodes;
    m_stats.totalBytesEncoded += static_cast<quint64>(data.size());
    ++m_stats.operationsByEncoding[static_cast<int>(encoding)];
    emit encodeComplete(result, encoding);
    return result;
}

/** @brief 从指定格式解码为原始字节 @param data 编码数据 @param encoding 源编码 @return 解码结果 */
QByteArray SerialDataEncoder::decode(const QByteArray &data, Encoding encoding)
{
    if (data.isEmpty()) return QByteArray();
    QByteArray result;
    try {
        switch (encoding) {
        case Encoding::Hex:              result = decodeHex(data); break;
        case Encoding::Base64:           result = QByteArray::fromBase64(data); break;
        case Encoding::Ascii:            result = data; break;
        case Encoding::Binary:           result = decodeBinary(data); break;
        case Encoding::UrlEncode:        result = decodeUrl(data); break;
        case Encoding::QuotedPrintable:  result = decodeQuotedPrintable(data); break;
        case Encoding::Base32:           result = decodeBase32(data); break;
        case Encoding::Base85:           result = decodeBase85(data); break;
        }
    } catch (...) {
        ++m_stats.decodeErrors;
        emit error(tr("解码失败: 内部异常"), encoding);
        return QByteArray();
    }
    if (result.isNull() && encoding != Encoding::Ascii) {
        ++m_stats.decodeErrors;
        emit error(tr("解码失败: 输入数据格式不匹配"), encoding);
        return QByteArray();
    }
    ++m_stats.totalDecodes;
    m_stats.totalBytesDecoded += static_cast<quint64>(result.size());
    ++m_stats.operationsByEncoding[static_cast<int>(encoding)];
    emit decodeComplete(result, encoding);
    return result;
}

/** @brief 编码为字符串形式 @param data 原始字节 @param encoding 目标编码 @return 编码后字符串 */
QString SerialDataEncoder::encodeToString(const QByteArray &data, Encoding encoding) const
{
    if (data.isEmpty()) return {};
    QByteArray result;
    switch (encoding) {
    case Encoding::Hex:              result = encodeHex(data); break;
    case Encoding::Base64:           result = data.toBase64(); break;
    case Encoding::Ascii:            result = data; break;
    case Encoding::Binary:           result = encodeBinary(data); break;
    case Encoding::UrlEncode:        result = encodeUrl(data); break;
    case Encoding::QuotedPrintable:  result = encodeQuotedPrintable(data); break;
    case Encoding::Base32:           result = encodeBase32(data); break;
    case Encoding::Base85:           result = encodeBase85(data); break;
    }
    return QString::fromLatin1(result);
}

// ── 自动检测与校验 ──

/** @brief 自动检测最可能的编码格式，启发式评分 @param data 待检测数据 @return 最可能的编码格式 */
SerialDataEncoder::Encoding SerialDataEncoder::detectEncoding(const QByteArray &data) const
{
    if (data.isEmpty()) return Encoding::Ascii;
    int scores[8] = {};
    if (isHexData(data))  scores[static_cast<int>(Encoding::Hex)] += 3;
    if (isBase64Data(data)) {
        scores[static_cast<int>(Encoding::Base64)] += 2;
        if (data.size() % 4 == 0) scores[static_cast<int>(Encoding::Base64)] += 2;
    }
    if (isBase32Data(data)) {
        scores[static_cast<int>(Encoding::Base32)] += 2;
        if (data.size() % 8 == 0) scores[static_cast<int>(Encoding::Base32)] += 1;
    }
    if (isBase85Data(data))       scores[static_cast<int>(Encoding::Base85)] += 1;
    if (isUrlEncodedData(data))   scores[static_cast<int>(Encoding::UrlEncode)] += 3;
    if (isQuotedPrintableData(data)) scores[static_cast<int>(Encoding::QuotedPrintable)] += 3;
    if (isBinaryData(data))       scores[static_cast<int>(Encoding::Binary)] += 4;
    int printable = 0;
    for (char c : data) { if (c >= 0x20 && c <= 0x7E) ++printable; }
    if (printable == data.size()) scores[static_cast<int>(Encoding::Ascii)] += 1;
    int bestIdx = static_cast<int>(Encoding::Ascii);
    for (int i = 0; i < 8; ++i) { if (scores[i] > scores[bestIdx]) bestIdx = i; }
    return static_cast<Encoding>(bestIdx);
}

/** @brief 检测数据是否符合指定编码格式 @param data 待检测数据 @param encoding 目标编码 @return true表示符合 */
bool SerialDataEncoder::isEncoded(const QByteArray &data, Encoding encoding) const
{
    if (data.isEmpty()) return true;
    switch (encoding) {
    case Encoding::Hex:              return isHexData(data);
    case Encoding::Base64:           return isBase64Data(data);
    case Encoding::Ascii:            return true;
    case Encoding::Binary:           return isBinaryData(data);
    case Encoding::UrlEncode:        return isUrlEncodedData(data);
    case Encoding::QuotedPrintable:  return isQuotedPrintableData(data);
    case Encoding::Base32:           return isBase32Data(data);
    case Encoding::Base85:           return isBase85Data(data);
    }
    return false;
}

// ── Hex 编解码 ──

/** @brief 十六进制编码，输出大写无分隔符 @param data 原始字节 @return Hex字符串 */
QByteArray SerialDataEncoder::encodeHex(const QByteArray &data) const
{
    return data.toHex().toUpper();
}

/** @brief 十六进制解码，支持空格/冒号/逗号/无分隔符 @param data Hex字符串 @return 原始字节 */
QByteArray SerialDataEncoder::decodeHex(const QByteArray &data) const
{
    QByteArray clean;
    clean.reserve(data.size());
    for (char c : data) {
        if (c != ' ' && c != ':' && c != '-' && c != ',' && c != ';')
            clean.append(c);
    }
    return QByteArray::fromHex(clean);
}

// ── Base32 编解码 (RFC 4648) ──

/** @brief Base32编码 @param data 原始字节 @return Base32字符串 */
QByteArray SerialDataEncoder::encodeBase32(const QByteArray &data) const
{
    static const char alpha[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ234567";
    QByteArray result;
    result.reserve((data.size() * 8 + 4) / 5);
    int bits = 0;
    quint32 buf = 0;
    for (char byte : data) {
        buf = (buf << 8) | static_cast<quint8>(byte);
        bits += 8;
        while (bits >= 5) {
            bits -= 5;
            result.append(alpha[(buf >> bits) & 0x1F]);
        }
    }
    if (bits > 0) result.append(alpha[(buf << (5 - bits)) & 0x1F]);
    int pad = (8 - (result.size() % 8)) % 8;
    result.append(QByteArray(pad, '='));
    return result;
}

/** @brief Base32解码 @param data Base32字符串 @return 原始字节 */
QByteArray SerialDataEncoder::decodeBase32(const QByteArray &data) const
{
    static quint8 lut[256];
    static bool init = false;
    if (!init) {
        std::fill(lut, lut + 256, quint8(0xFF));
        for (int i = 0; i < 26; ++i) lut[static_cast<int>('A') + i] = quint8(i);
        for (int i = 0; i < 6; ++i)  lut[static_cast<int>('2') + i] = quint8(26 + i);
        for (int i = 0; i < 26; ++i) lut[static_cast<int>('a') + i] = quint8(i);
        init = true;
    }
    QByteArray result;
    result.reserve(data.size() * 5 / 8);
    int bits = 0;
    quint32 buf = 0;
    for (char c : data) {
        quint8 val = lut[static_cast<quint8>(c)];
        if (val == 0xFF) {
            if (c == '=' || c == ' ' || c == '\n' || c == '\r' || c == '\t') continue;
            return QByteArray();
        }
        buf = (buf << 5) | val;
        bits += 5;
        if (bits >= 8) { bits -= 8; result.append(char((buf >> bits) & 0xFF)); }
    }
    return result;
}

// ── Base85 (Ascii85) 编解码 ──

/** @brief Base85(Ascii85)编码，以<~开头~>结尾 @param data 原始字节 @return Ascii85字符串 */
QByteArray SerialDataEncoder::encodeBase85(const QByteArray &data) const
{
    QByteArray result;
    result.reserve(data.size() * 5 / 4 + 4);
    result.append("<~");
    static const quint32 p85[] = {52200625u, 614125u, 7225u, 85u, 1u};
    int i = 0;
    while (i + 3 < data.size()) {
        quint32 val = (quint32(quint8(data[i])) << 24) | (quint32(quint8(data[i+1])) << 16)
                    | (quint32(quint8(data[i+2])) << 8) | quint32(quint8(data[i+3]));
        if (val == 0) { result.append('z'); }
        else { char blk[5]; for (int j = 0; j < 5; ++j) blk[j] = char((val / p85[j]) % 85 + 33); result.append(blk, 5); }
        i += 4;
    }
    int rem = data.size() - i;
    if (rem > 0) {
        quint32 val = 0;
        for (int j = 0; j < rem; ++j) val = (val << 8) | quint8(data[i + j]);
        val <<= (4 - rem) * 8;
        for (int j = 0; j < rem + 1; ++j) result.append(char((val / p85[j]) % 85 + 33));
    }
    result.append("~>");
    return result;
}

/** @brief Base85(Ascii85)解码 @param data Ascii85字符串 @return 原始字节 */
QByteArray SerialDataEncoder::decodeBase85(const QByteArray &data) const
{
    QByteArray payload = data;
    if (payload.startsWith("<~")) payload = payload.mid(2);
    if (payload.endsWith("~>"))   payload.chop(2);
    QByteArray clean;
    clean.reserve(payload.size());
    for (char c : payload) { if (c != ' ' && c != '\n' && c != '\r' && c != '\t') clean.append(c); }
    QByteArray result;
    result.reserve(clean.size() * 4 / 5 + 4);
    int i = 0;
    while (i < clean.size()) {
        if (clean[i] == 'z') { result.append(4, '\0'); ++i; continue; }
        int bs = qMin(5, clean.size() - i);
        quint32 val = 0;
        for (int j = 0; j < 5; ++j) val = val * 85 + ((j < bs) ? (quint8(clean[i+j]) - 33u) : 84u);
        for (int j = 3; j >= 4 - (bs - 1); --j) result.append(char((val >> (j * 8)) & 0xFF));
        i += bs;
    }
    return result;
}

// ── URL 百分号编码/解码 ──

/** @brief URL编码(RFC 3986) @param data 原始字节 @return URL编码字符串 */
QByteArray SerialDataEncoder::encodeUrl(const QByteArray &data) const
{
    QByteArray result;
    result.reserve(data.size() * 3);
    for (char c : data) {
        quint8 b = quint8(c);
        if ((b >= 'A' && b <= 'Z') || (b >= 'a' && b <= 'z') || (b >= '0' && b <= '9')
            || b == '-' || b == '_' || b == '.' || b == '~') {
            result.append(c);
        } else {
            result.append('%');
            result.append(QByteArray(1, c).toHex().toUpper());
        }
    }
    return result;
}

/** @brief URL百分号解码 @param data URL编码字符串 @return 原始字节 */
QByteArray SerialDataEncoder::decodeUrl(const QByteArray &data) const
{
    QByteArray result;
    result.reserve(data.size());
    int i = 0;
    while (i < data.size()) {
        if (data[i] == '%' && i + 2 < data.size()) {
            QByteArray dec = QByteArray::fromHex(data.mid(i + 1, 2));
            if (!dec.isEmpty()) { result.append(dec); i += 3; continue; }
        }
        result.append(data[i++]);
    }
    return result;
}

// ── Quoted-Printable 编解码 ──

/** @brief Quoted-Printable编码(RFC 2045) @param data 原始字节 @return QP字符串 */
QByteArray SerialDataEncoder::encodeQuotedPrintable(const QByteArray &data) const
{
    QByteArray result;
    result.reserve(data.size() * 3);
    int lineLen = 0;
    for (char c : data) {
        quint8 b = quint8(c);
        bool esc = (b == '=' || b < 0x20 || b > 0x7E);
        QByteArray chunk;
        if (esc) {
            chunk = "=" + QByteArray(1, char(b)).toHex().toUpper();
        } else {
            chunk = QByteArray(1, c);
        }
        if (lineLen + chunk.size() > 75) { result.append("=\r\n"); lineLen = 0; }
        result.append(chunk);
        lineLen += chunk.size();
    }
    return result;
}

/** @brief Quoted-Printable解码 @param data QP字符串 @return 原始字节 */
QByteArray SerialDataEncoder::decodeQuotedPrintable(const QByteArray &data) const
{
    QByteArray result;
    result.reserve(data.size());
    int i = 0;
    while (i < data.size()) {
        if (data[i] == '=') {
            if (i + 2 < data.size()) {
                QByteArray dec = QByteArray::fromHex(data.mid(i + 1, 2));
                if (!dec.isEmpty()) { result.append(dec); i += 3; continue; }
            }
            if (i + 1 < data.size() && data[i+1] == '\r') { i += (i+2 < data.size() && data[i+2] == '\n') ? 3 : 2; continue; }
            if (i + 1 < data.size() && data[i+1] == '\n') { i += 2; continue; }
        }
        result.append(data[i++]);
    }
    return result;
}

// ── Binary 编解码 ──

/** @brief 二进制编码，空格分隔 @param data 原始字节 @return 0/1字符串 */
QByteArray SerialDataEncoder::encodeBinary(const QByteArray &data) const
{
    QByteArray result;
    result.reserve(data.size() * 9);
    for (int i = 0; i < data.size(); ++i) {
        if (i > 0) result.append(' ');
        quint8 b = quint8(data[i]);
        for (int bit = 7; bit >= 0; --bit) result.append((b >> bit) & 1 ? '1' : '0');
    }
    return result;
}

/** @brief 二进制解码 @param data 0/1字符串 @return 原始字节 */
QByteArray SerialDataEncoder::decodeBinary(const QByteArray &data) const
{
    QByteArray clean;
    clean.reserve(data.size());
    for (char c : data) { if (c == '0' || c == '1') clean.append(c); }
    QByteArray result;
    int cnt = clean.size() / 8;
    result.reserve(cnt);
    for (int i = 0; i < cnt; ++i) {
        quint8 b = 0;
        for (int bit = 0; bit < 8; ++bit) b = (b << 1) | (clean[i*8+bit] == '1' ? 1 : 0);
        result.append(char(b));
    }
    return result;
}

// ── 有效性检测 ──

/** @brief 检测是否为合法Hex格式 @param data 待检测 @return true合法 */
bool SerialDataEncoder::isHexData(const QByteArray &data) const
{
    int hc = 0;
    for (char c : data) {
        if ((c >= '0' && c <= '9') || (c >= 'A' && c <= 'F') || (c >= 'a' && c <= 'f')) { ++hc; }
        else if (c != ' ' && c != ':' && c != '-' && c != ',' && c != ';') return false;
    }
    return hc >= 2 && hc % 2 == 0;
}

/** @brief 检测是否为合法Base64格式 @param data 待检测 @return true合法 */
bool SerialDataEncoder::isBase64Data(const QByteArray &data) const
{
    if (data.size() < 4) return false;
    int pad = 0;
    for (char c : data) {
        if ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || c == '+' || c == '/') continue;
        if (c == '=') { if (++pad > 2) return false; continue; }
        return false;
    }
    return true;
}

/** @brief 检测是否为合法Base32格式 @param data 待检测 @return true合法 */
bool SerialDataEncoder::isBase32Data(const QByteArray &data) const
{
    if (data.size() < 2) return false;
    for (char c : data) {
        if ((c >= 'A' && c <= 'Z') || (c >= '2' && c <= '7') || (c >= 'a' && c <= 'z') || c == '=' || c == ' ' || c == '\n') continue;
        return false;
    }
    return true;
}

/** @brief 检测是否为合法Ascii85格式 @param data 待检测 @return true合法 */
bool SerialDataEncoder::isBase85Data(const QByteArray &data) const
{
    QByteArray p = data;
    if (p.startsWith("<~")) p = p.mid(2);
    if (p.endsWith("~>"))   p.chop(2);
    if (p.isEmpty()) return false;
    for (char c : p) {
        quint8 b = quint8(c);
        if (c == ' ' || c == '\n' || c == '\r' || c == '\t' || c == 'z') continue;
        if (b < 33 || b > 117) return false;
    }
    return true;
}

/** @brief 检测是否含URL编码%XX @param data 待检测 @return true含%XX */
bool SerialDataEncoder::isUrlEncodedData(const QByteArray &data) const
{
    int cnt = 0;
    for (int i = 0; i < data.size(); ++i) {
        if (data[i] != '%') continue;
        if (i + 2 >= data.size()) return false;
        auto hOk = [](char c) { return (c >= '0' && c <= '9') || (c >= 'A' && c <= 'F') || (c >= 'a' && c <= 'f'); };
        if (!hOk(data[i+1]) || !hOk(data[i+2])) return false;
        ++cnt;
    }
    return cnt > 0;
}

/** @brief 检测是否含QP格式=XX @param data 待检测 @return true含=XX */
bool SerialDataEncoder::isQuotedPrintableData(const QByteArray &data) const
{
    int cnt = 0;
    for (int i = 0; i < data.size(); ++i) {
        if (data[i] != '=' || i + 2 >= data.size()) continue;
        auto hOk = [](char c) { return (c >= '0' && c <= '9') || (c >= 'A' && c <= 'F'); };
        if (hOk(data[i+1]) && hOk(data[i+2])) ++cnt;
    }
    return cnt > 0;
}

/** @brief 检测是否为纯二进制字符串 @param data 待检测 @return true合法 */
bool SerialDataEncoder::isBinaryData(const QByteArray &data) const
{
    int bc = 0;
    for (char c : data) {
        if (c == '0' || c == '1') ++bc;
        else if (c != ' ') return false;
    }
    return bc >= 8;
}

// ── 元数据接口 ──

/** @brief 获取编码格式标准名称 @param encoding 编码枚举 @return 格式名称 */
QString SerialDataEncoder::encodingName(Encoding encoding)
{
    switch (encoding) {
    case Encoding::Hex:              return QStringLiteral("Hex");
    case Encoding::Base64:           return QStringLiteral("Base64");
    case Encoding::Ascii:            return QStringLiteral("ASCII");
    case Encoding::Binary:           return QStringLiteral("Binary");
    case Encoding::UrlEncode:        return QStringLiteral("URL");
    case Encoding::QuotedPrintable:  return QStringLiteral("Quoted-Printable");
    case Encoding::Base32:           return QStringLiteral("Base32");
    case Encoding::Base85:           return QStringLiteral("Base85");
    }
    return QStringLiteral("Unknown");
}

/** @brief 获取编码格式中文描述 @param encoding 编码枚举 @return 中文描述 */
QString SerialDataEncoder::encodingDescription(Encoding encoding)
{
    switch (encoding) {
    case Encoding::Hex:              return tr("十六进制编码，每字节转为2位HEX字符");
    case Encoding::Base64:           return tr("Base64编码(RFC 4648)，常用于二进制文本传输");
    case Encoding::Ascii:            return tr("ASCII明文，无编码转换");
    case Encoding::Binary:           return tr("二进制字符串，每字节展开为8位0/1");
    case Encoding::UrlEncode:        return tr("URL百分号编码(RFC 3986)，非安全字符转为%XX");
    case Encoding::QuotedPrintable:  return tr("Quoted-Printable编码(RFC 2045)，邮件附件标准");
    case Encoding::Base32:           return tr("Base32编码(RFC 4648)，不区分大小写，适合人工抄录");
    case Encoding::Base85:           return tr("Base85/Ascii85编码，PDF PostScript标准");
    }
    return QString();
}

// ── 统计方法见 SerialDataEncoderStats.cpp ──
