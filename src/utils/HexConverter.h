/**
 * @file HexConverter.h
 * @brief HEX编码/解码工具 — 纯函数，无状态，全局复用
 *
 * 提供HEX字符串与QByteArray之间的双向转换、校验和计算。
 * 所有方法均为静态函数，无需实例化。
 */
#ifndef HEXCONVERTER_H
#define HEXCONVERTER_H

#include <QByteArray>
#include <QString>

// HEX编码解码工具 - 纯静态方法，不需要实例化
namespace HexConverter {

// 将字节数组转为HEX字符串 "AA BB CC"
inline QString toHexString(const QByteArray& data, char separator = ' ')
{
    QString result;
    result.reserve(data.size() * 3);
    for (int i = 0; i < data.size(); ++i) {
        if (i > 0) result += separator;
        result += QString("%1").arg(static_cast<unsigned char>(data[i]), 2, 16, QChar('0')).toUpper();
    }
    return result;
}

// 将HEX字符串转为字节数组
// 支持格式: "AA BB CC" / "AABBCC" / "AA,BB,CC" / "0xAA 0xBB"
inline QByteArray fromHexString(const QString& hexStr)
{
    // 移除所有分隔符和前缀
    QString clean = hexStr;
    clean.remove(' ');
    clean.remove(',');
    clean.remove("0x", Qt::CaseInsensitive);
    clean.remove('\n');
    clean.remove('\r');
    clean.remove('\t');

    // 必须是偶数个字符
    if (clean.length() % 2 != 0) {
        return QByteArray();
    }

    QByteArray result;
    result.reserve(clean.length() / 2);
    for (int i = 0; i < clean.length(); i += 2) {
        bool ok;
        unsigned char byte = static_cast<unsigned char>(
            clean.mid(i, 2).toUInt(&ok, 16));
        if (!ok) {
            return QByteArray();  // 解析失败
        }
        result.append(byte);
    }
    return result;
}

// 检查字符串是否是有效的HEX格式
inline bool isValidHex(const QString& str)
{
    return !fromHexString(str).isNull();
}

} // namespace HexConverter

#endif // HEXCONVERTER_H
