/**
 * @file LzwCodec2.cpp
 * @brief LZW编解码器(增强版)实现
 */

#include "LzwCodec2.h"

#include <QElapsedTimer>
#include <QDataStream>
#include <QIODevice>
#include <QMap>

// ═══════════════════════════════════════════════════════════
// 构造 / 析构
// ═══════════════════════════════════════════════════════════

LzwCodec2::LzwCodec2(QObject* parent)
    : QObject(parent)
{
}

LzwCodec2::~LzwCodec2() = default;

// ═══════════════════════════════════════════════════════════
// 配置
// ═══════════════════════════════════════════════════════════

void LzwCodec2::setMaxCodeBits(int bits)
{
    m_maxCodeBits = std::min({std::max(bits, 9), 16});
}

int LzwCodec2::maxCodeBits() const
{
    return m_maxCodeBits;
}

void LzwCodec2::setAutoReset(bool enable)
{
    m_autoReset = enable;
}

// ═══════════════════════════════════════════════════════════
// 编码
// ═══════════════════════════════════════════════════════════

QByteArray LzwCodec2::encode(const QByteArray& data)
{
    if (data.isEmpty()) {
        m_stats.totalEncodes += 1;
        emit encoded({}, 0.0);
        return {};
    }

    QElapsedTimer timer;
    timer.start();

    // 初始化字典: 单字节条目 0~255
    QMap<QByteArray, quint32> dict;
    for (quint32 i = 0; i < 256; ++i) {
        dict[QByteArray(1, static_cast<char>(i))] = i;
    }

    quint32 nextCode = 256;
    const quint32 maxCode = (1U << m_maxCodeBits) - 1;
    QVector<quint32> output;

    QByteArray w;
    w.reserve(16);

    for (int i = 0; i < data.size(); ++i) {
        const QByteArray wc = w + data[i];
        if (dict.contains(wc)) {
            w = wc;
        } else {
            output.append(dict.value(w));
            if (nextCode <= maxCode) {
                dict[wc] = nextCode++;
            } else if (m_autoReset) {
                // 字典满, 重置
                dict.clear();
                for (quint32 j = 0; j < 256; ++j) {
                    dict[QByteArray(1, static_cast<char>(j))] = j;
                }
                nextCode = 256;
                m_stats.dictResets += 1;
            }
            w = QByteArray(1, data[i]);
        }
    }

    if (!w.isEmpty()) {
        output.append(dict.value(w, 0));
    }

    const int codeSize = std::min({std::max(calcBits(nextCode), 9), m_maxCodeBits});
    QByteArray packed = packCodes(output, codeSize);

    // 头部: 原始长度 + 编码数 + 位宽 + 自动重置标志
    QByteArray header;
    QDataStream hs(&header, QIODevice::WriteOnly);
    hs << static_cast<quint32>(data.size());
    hs << static_cast<quint32>(output.size());
    hs << static_cast<quint16>(codeSize);
    hs << static_cast<quint8>(m_autoReset ? 1 : 0);

    QByteArray result = header + packed;

    m_stats.totalEncodes += 1;
    m_stats.totalBytesIn += static_cast<quint64>(data.size());
    m_stats.totalBytesOut += static_cast<quint64>(result.size());

    const double ratio = static_cast<double>(result.size()) /
                         static_cast<double>(std::max(data.size(), 1));
    if (m_stats.totalEncodes == 1) {
        m_stats.avgRatio = ratio;
    } else {
        m_stats.avgRatio += (ratio - m_stats.avgRatio) /
                            static_cast<double>(m_stats.totalEncodes);
    }

    emit encoded(result, ratio);
    return result;
}

// ═══════════════════════════════════════════════════════════
// 解码
// ═══════════════════════════════════════════════════════════

QByteArray LzwCodec2::decode(const QByteArray& data)
{
    // 头部11字节: 4 + 4 + 2 + 1
    if (data.size() < 11) {
        emit error(tr("LZW2解码错误: 数据太短, 缺少头部"));
        return {};
    }

    QDataStream hs(data);
    quint32 originalSize = 0;
    quint32 codeCount = 0;
    quint16 codeSize = 0;
    quint8 autoResetFlag = 0;
    hs >> originalSize >> codeCount >> codeSize >> autoResetFlag;

    if (codeSize < 9 || codeSize > 16) {
        emit error(tr("LZW2解码错误: 无效编码位宽 %1").arg(codeSize));
        return {};
    }

    const QByteArray packedData = data.mid(11);
    QVector<quint32> codes = unpackCodes(packedData, codeSize,
                                         static_cast<int>(codeCount));
    if (codes.isEmpty() && codeCount > 0) {
        emit error(tr("LZW2解码错误: 编码解包失败"));
        return {};
    }

    // 初始化字典
    QVector<QByteArray> dict(256);
    for (int i = 0; i < 256; ++i) {
        dict[i] = QByteArray(1, static_cast<char>(i));
    }

    quint32 nextCode = 256;
    const quint32 maxCode = (1U << codeSize) - 1;
    QByteArray result;
    result.reserve(static_cast<int>(originalSize));

    if (codes.isEmpty()) {
        m_stats.totalDecodes += 1;
        emit decoded({});
        return {};
    }

    quint32 prevCode = codes[0];
    if (prevCode >= static_cast<quint32>(dict.size())) {
        emit error(tr("LZW2解码错误: 首编码超出字典范围"));
        return {};
    }

    result.append(dict[prevCode]);

    for (int i = 1; i < codes.size(); ++i) {
        const quint32 code = codes[i];
        QByteArray entry;

        if (code < nextCode) {
            entry = dict[static_cast<int>(code)];
        } else if (code == nextCode) {
            entry = dict[static_cast<int>(prevCode)] +
                    QByteArray(1, dict[static_cast<int>(prevCode)][0]);
        } else {
            emit error(tr("LZW2解码错误: 编码 %1 超出字典范围").arg(code));
            return {};
        }

        result.append(entry);

        if (nextCode <= maxCode) {
            dict.append(dict[static_cast<int>(prevCode)] +
                        QByteArray(1, entry[0]));
            nextCode++;
        } else if (autoResetFlag) {
            dict.resize(256);
            nextCode = 256;
        }

        prevCode = code;
    }

    m_stats.totalDecodes += 1;
    emit decoded(result);
    return result;
}

// ═══════════════════════════════════════════════════════════
// 统计
// ═══════════════════════════════════════════════════════════

LzwCodec2::Stats LzwCodec2::stats() const
{
    return m_stats;
}

void LzwCodec2::resetStatistics()
{
    m_stats = Stats{};
}

// ═══════════════════════════════════════════════════════════
// 内部辅助
// ═══════════════════════════════════════════════════════════

QByteArray LzwCodec2::packCodes(const QVector<quint32>& codes,
                                 int codeSize) const
{
    QByteArray result;
    result.reserve(static_cast<int>(
        static_cast<quint64>(codes.size()) *
        static_cast<quint64>(codeSize) / 8ULL + 1ULL));

    quint64 buffer = 0;
    int bits = 0;
    for (quint32 code : codes) {
        buffer = (buffer << codeSize) | static_cast<quint64>(code);
        bits += codeSize;
        while (bits >= 8) {
            bits -= 8;
            result.append(static_cast<char>((buffer >> bits) & 0xFF));
        }
    }
    if (bits > 0) {
        result.append(static_cast<char>((buffer << (8 - bits)) & 0xFF));
    }
    return result;
}

QVector<quint32> LzwCodec2::unpackCodes(const QByteArray& data,
                                         int codeSize,
                                         int codeCount) const
{
    QVector<quint32> result;
    result.reserve(codeCount);

    quint64 buffer = 0;
    int bits = 0;
    int bytePos = 0;

    for (int i = 0; i < codeCount; ++i) {
        while (bits < codeSize && bytePos < data.size()) {
            buffer = (buffer << 8) |
                     static_cast<quint8>(data[bytePos++]);
            bits += 8;
        }
        if (bits < codeSize) break;
        bits -= codeSize;
        result.append(static_cast<quint32>(
            (buffer >> bits) & ((1ULL << codeSize) - 1)));
    }
    return result;
}

int LzwCodec2::calcBits(quint32 maxCode) const
{
    int bits = 1;
    quint32 val = 1;
    while (val < maxCode) { val <<= 1; bits++; }
    return std::min({std::max(bits, 9), m_maxCodeBits});
}
