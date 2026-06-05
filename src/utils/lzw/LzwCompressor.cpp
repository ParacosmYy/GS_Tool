/**
 * @file LzwCompressor.cpp
 * @brief LZW压缩引擎实现 — 基于字典的自适应压缩/解压
 */

#include "LzwCompressor.h"

#include <QElapsedTimer>
#include <QDataStream>
#include <QIODevice>
#include <QMap>

// ═══════════════════════════════════════════════════════════
// 构造 / 析构
// ═══════════════════════════════════════════════════════════

LzwCompressor::LzwCompressor(QObject* parent)
    : QObject(parent)
{
}

LzwCompressor::~LzwCompressor() = default;

// ═══════════════════════════════════════════════════════════
// 配置
// ═══════════════════════════════════════════════════════════

void LzwCompressor::setMaxDictSize(int maxSize)
{
    m_maxDictSize = qMax(256, maxSize);
}

int LzwCompressor::maxDictSize() const
{
    return m_maxDictSize;
}

// ═══════════════════════════════════════════════════════════
// 压缩
// ═══════════════════════════════════════════════════════════

QByteArray LzwCompressor::compress(const QByteArray& data)
{
    if (data.isEmpty()) {
        m_stats.totalCompressions += 1;
        emit compressed({}, 0.0);
        return {};
    }

    // 初始化字典: 单字节条目 0~255
    QMap<QByteArray, quint32> dict;
    for (quint32 i = 0; i < 256; ++i) {
        dict[QByteArray(1, static_cast<char>(i))] = i;
    }

    quint32 nextCode = 256;
    QVector<quint32> output;

    QByteArray w;
    w.reserve(16);

    for (int i = 0; i < data.size(); ++i) {
        const QByteArray wc = w + data[i];

        if (dict.contains(wc)) {
            w = wc;
        } else {
            output.append(dict.value(w));

            if (static_cast<int>(nextCode) < m_maxDictSize) {
                dict[wc] = nextCode++;
            }

            w = QByteArray(1, data[i]);
        }
    }

    // 输出最后一段
    if (!w.isEmpty()) {
        output.append(dict.value(w, 0));
    }

    // 计算编码位数
    const int codeSize = qMax(9, calcCodeBits(nextCode));

    // 打包输出
    QByteArray header;
    QDataStream hs(&header, QIODevice::WriteOnly);
    hs << static_cast<quint32>(data.size());
    hs << static_cast<quint32>(output.size());
    hs << static_cast<quint16>(codeSize);

    QByteArray packed = packCodes(output, codeSize);
    QByteArray result = header + packed;

    // 更新统计
    m_stats.totalCompressions += 1;
    const double ratio = static_cast<double>(result.size()) /
                         static_cast<double>(data.size());
    updateAvgRatio(ratio);

    emit compressed(result, ratio);
    return result;
}

// ═══════════════════════════════════════════════════════════
// 解压
// ═══════════════════════════════════════════════════════════

QByteArray LzwCompressor::decompress(const QByteArray& data)
{
    if (data.size() < 10) {
        emit error(tr("LZW解压错误: 数据太短, 缺少头部"));
        return {};
    }

    // 读取头部
    QDataStream hs(data);
    quint32 originalSize = 0;
    quint32 codeCount = 0;
    quint16 codeSize = 0;
    hs >> originalSize >> codeCount >> codeSize;

    if (codeSize < 1 || codeSize > 32) {
        emit error(tr("LZW解压错误: 无效编码位数 %1").arg(codeSize));
        return {};
    }

    const QByteArray packedData = data.mid(10);
    QVector<quint32> codes = unpackCodes(packedData, codeSize,
                                         static_cast<int>(codeCount));

    if (codes.isEmpty() && codeCount > 0) {
        emit error(tr("LZW解压错误: 编码解包失败"));
        return {};
    }

    // 初始化字典
    QVector<QByteArray> dict(256);
    for (int i = 0; i < 256; ++i) {
        dict[i] = QByteArray(1, static_cast<char>(i));
    }

    quint32 nextCode = 256;
    QByteArray result;
    result.reserve(static_cast<int>(originalSize));

    if (codes.isEmpty()) {
        m_stats.totalDecompressions += 1;
        emit decompressed({});
        return {};
    }

    quint32 prevCode = codes[0];
    if (prevCode >= static_cast<quint32>(dict.size())) {
        emit error(tr("LZW解压错误: 首编码超出字典范围"));
        return {};
    }

    result.append(dict[prevCode]);

    for (int i = 1; i < codes.size(); ++i) {
        const quint32 code = codes[i];

        QByteArray entry;
        if (code < nextCode) {
            entry = dict[code];
        } else if (code == nextCode) {
            entry = dict[prevCode] +
                    QByteArray(1, dict[prevCode][0]);
        } else {
            emit error(tr("LZW解压错误: 编码 %1 超出字典范围").arg(code));
            return {};
        }

        result.append(entry);

        if (static_cast<int>(nextCode) < m_maxDictSize) {
            dict.append(dict[prevCode] +
                        QByteArray(1, entry[0]));
            nextCode++;
        }

        prevCode = code;
    }

    m_stats.totalDecompressions += 1;
    emit decompressed(result);
    return result;
}

// ═══════════════════════════════════════════════════════════
// 统计
// ═══════════════════════════════════════════════════════════

LzwCompressor::Stats LzwCompressor::stats() const
{
    return m_stats;
}

void LzwCompressor::resetStatistics()
{
    m_stats = Stats{};
}

// ═══════════════════════════════════════════════════════════
// 内部辅助
// ═══════════════════════════════════════════════════════════

QByteArray LzwCompressor::packCodes(const QVector<quint32>& codes,
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
            result.append(static_cast<char>(
                (buffer >> bits) & 0xFF));
        }
    }

    if (bits > 0) {
        result.append(static_cast<char>(
            (buffer << (8 - bits)) & 0xFF));
    }

    return result;
}

QVector<quint32> LzwCompressor::unpackCodes(const QByteArray& data,
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

        if (bits < codeSize) {
            break;
        }

        bits -= codeSize;
        const quint32 code = static_cast<quint32>(
            (buffer >> bits) & ((1ULL << codeSize) - 1));
        result.append(code);
    }

    return result;
}

int LzwCompressor::calcCodeBits(quint32 maxCode) const
{
    int bits = 1;
    quint32 val = 1;
    while (val < maxCode) {
        val <<= 1;
        bits++;
    }
    return qMax(bits, 9);
}

void LzwCompressor::updateAvgRatio(double ratio)
{
    const auto n = m_stats.totalCompressions;
    if (n == 1) {
        m_stats.avgRatio = ratio;
    } else {
        m_stats.avgRatio =
            m_stats.avgRatio * static_cast<double>(n - 1) /
                static_cast<double>(n) +
            ratio / static_cast<double>(n);
    }
}
