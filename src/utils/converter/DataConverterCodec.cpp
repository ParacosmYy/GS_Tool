/**
 * @file DataConverterCodec.cpp
 * @brief 数据格式转换引擎 - 编解码方法实现
 *
 * 从 DataConverter.cpp 拆分而来，包含各格式的解码(decodeToRaw)
 * 和编码(encodeFromRaw)核心转换逻辑。
 *
 * 输入验证增强:
 * - Hex: 仅允许 0-9, A-F, a-f 和空格，非法字符返回空并递增错误计数
 * - Decimal: 仅允许 0-9 和空格，非法字符返回空并递增错误计数
 * - Binary: 仅允许 0-1 和空格，非法字符返回空并递增错误计数
 */

#include "utils/converter/DataConverter.h"
#include <QUrl>

namespace {
/**
 * @brief 验证字符串中每个字符是否都属于指定合法字符集
 * @param text 待验证字符串
 * @param validChars 合法字符集合(如"0123456789ABCDEFabcdef ")
 * @return true=所有字符都合法
 */
bool isValidCharSet(const QString& text, const QString& validChars)
{
    for (const QChar& ch : text) {
        if (!validChars.contains(ch)) return false;
    }
    return true;
}
} // anonymous namespace

/** @brief 从指定格式解码为原始字节(含输入验证) @param input 输入数据 @param from 源格式 @return 解码后的原始字节数据，验证失败返回空 */
QByteArray DataConverter::decodeToRaw(const QByteArray &input, Format from) const
{
    switch (from) {
    case Hex: {
        QString hex = QString::fromUtf8(input).simplified();
        hex.remove(' ');
        /* 验证十六进制输入: 仅允许 0-9, A-F, a-f */
        if (!isValidCharSet(hex, QStringLiteral("0123456789ABCDEFabcdef"))) {
            m_lastValidationError = tr("十六进制输入包含非法字符，仅允许 0-9, A-F, a-f");
            ++m_totalErrors;
            return QByteArray();
        }
        m_lastValidationError.clear();
        return QByteArray::fromHex(hex.toUtf8());
    }
    case Ascii:
        return input;
    case Base64:
        return QByteArray::fromBase64(input);
    case UrlEncode:
        return QUrl::fromPercentEncoding(input).toUtf8();
    case Binary: {
        QString text = QString::fromUtf8(input).simplified();
        /* 验证二进制输入: 仅允许 0, 1 和空格(已simplified移除多余空格) */
        if (!isValidCharSet(text, QStringLiteral("01 "))) {
            m_lastValidationError = tr("二进制输入包含非法字符，仅允许 0 和 1");
            ++m_totalErrors;
            return QByteArray();
        }
        m_lastValidationError.clear();
        QByteArray result;
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
        QString text = QString::fromUtf8(input).simplified();
        /* 验证十进制输入: 仅允许 0-9 和空格 */
        if (!isValidCharSet(text, QStringLiteral("0123456789 "))) {
            m_lastValidationError = tr("十进制输入包含非法字符，仅允许 0-9");
            ++m_totalErrors;
            return QByteArray();
        }
        m_lastValidationError.clear();
        QByteArray result;
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
        QString text = QString::fromUtf8(input).simplified();
        /* 验证八进制输入: 仅允许 0-7 和空格 */
        if (!isValidCharSet(text, QStringLiteral("01234567 "))) {
            m_lastValidationError = tr("八进制输入包含非法字符，仅允许 0-7");
            ++m_totalErrors;
            return QByteArray();
        }
        m_lastValidationError.clear();
        QByteArray result;
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
