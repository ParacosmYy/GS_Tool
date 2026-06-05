/**
 * @file LempelZivWelch.cpp
 * @brief LZW自适应字典压缩编解码器实现
 */

#include "LempelZivWelch.h"

#include <QElapsedTimer>
#include <QDataStream>
#include <QIODevice>
#include <QMap>

// ═══════════════════════════════════════════════════════════
// 构造 / 析构
// ═══════════════════════════════════════════════════════════

LempelZivWelch::LempelZivWelch(QObject* parent)
    : QObject(parent)
{
}

LempelZivWelch::~LempelZivWelch() = default;

// ═══════════════════════════════════════════════════════════
// 配置
// ═══════════════════════════════════════════════════════════

void LempelZivWelch::setDictSize(int size)
{
    m_dictSize = qMax(256, size);
}

int LempelZivWelch::dictSize() const
{
    return m_dictSize;
}

// ═══════════════════════════════════════════════════════════
// 压缩
// ═══════════════════════════════════════════════════════════

QByteArray LempelZivWelch::compress(const QByteArray& data)
{
    QElapsedTimer timer;
    timer.start();

    if (data.isEmpty()) {
        m_stats.totalCompressions += 1;
        emit compressed(0.0);
        return {};
    }

    // 初始化字典: 单字节条目 0~255
    QMap<QByteArray, quint32> dict;
    for (quint32 i = 0; i < 256; ++i) {
        dict[QByteArray(1, static_cast<char>(i))] = i;
    }

    quint32 nextCode = 256;
    QVector<quint32> output;
    output.reserve(data.size());

    QByteArray w;
    w.reserve(16);

    for (int i = 0; i < data.size(); ++i) {
        const QByteArray wc = w + data[i];

        if (dict.contains(wc)) {
            w = wc;
        } else {
            output.append(dict.value(w));

            if (static_cast<int>(nextCode) < m_dictSize) {
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

    // 打包输出: 头部(原始大小 + 编码数 + 编码位数) + 编码数据
    QByteArray header;
    QDataStream hs(&header, QIODevice::WriteOnly);
    hs << static_cast<quint32>(data.size());
    hs << static_cast<quint32>(output.size());
    hs << static_cast<quint16>(codeSize);

    QByteArray packed = packCodes(output, codeSize);
    QByteArray result = header + packed;

    // 更新统计
    m_stats.totalCompressions += 1;
    m_stats.totalBytesIn += static_cast<quint64>(data.size());
    m_stats.totalBytesOut += static_cast<quint64>(result.size());
    const double ratio = static_cast<double>(result.size()) /
                         qMax(1.0, static_cast<double>(data.size()));
    const double elapsedMs = static_cast<double>(timer.elapsed());
    updateAvgTime(elapsedMs);

    emit compressed(ratio);
    return result;
}

// ═══════════════════════════════════════════════════════════
// 解压
// ═══════════════════════════════════════════════════════════

QByteArray LempelZivWelch::decompress(const QByteArray& data)
{
    QElapsedTimer timer;
    timer.start();

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
        emit decompressed(0);
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

        if (static_cast<int>(nextCode) < m_dictSize) {
            dict.append(dict[prevCode] +
                        QByteArray(1, entry[0]));
            nextCode++;
        }

        prevCode = code;
    }

    m_stats.totalDecompressions += 1;
    m_stats.totalBytesIn += static_cast<quint64>(data.size());
    m_stats.totalBytesOut += static_cast<quint64>(result.size());
    const double elapsedMs = static_cast<double>(timer.elapsed());
    updateAvgTime(elapsedMs);

    emit decompressed(result.size());
    return result;
}

// ═══════════════════════════════════════════════════════════
// 统计
// ═══════════════════════════════════════════════════════════

LempelZivWelch::Stats LempelZivWelch::stats() const
{
    return m_stats;
}

void LempelZivWelch::resetStatistics()
{
    m_stats = Stats{};
}

// ═══════════════════════════════════════════════════════════
// 内部辅助
// ═══════════════════════════════════════════════════════════

QByteArray LempelZivWelch::packCodes(const QVector<quint32>& codes,
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

QVector<quint32> LempelZivWelch::unpackCodes(const QByteArray& data,
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

int LempelZivWelch::calcCodeBits(quint32 maxCode) const
{
    int bits = 1;
    quint32 val = 1;
    while (val < maxCode) {
        val <<= 1;
        bits++;
    }
    return qMax(bits, 9);
}

void LempelZivWelch::updateAvgTime(double elapsedMs) const
{
    const auto total = m_stats.totalCompressions + m_stats.totalDecompressions;
    if (total <= 1) {
        m_stats.avgProcessingTimeMs = elapsedMs;
    } else {
        m_stats.avgProcessingTimeMs =
            m_stats.avgProcessingTimeMs *
                static_cast<double>(total - 1) / static_cast<double>(total) +
            elapsedMs / static_cast<double>(total);
    }
}
