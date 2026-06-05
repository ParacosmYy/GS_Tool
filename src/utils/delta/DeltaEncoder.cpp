/**
 * @file DeltaEncoder.cpp
 * @brief Delta增量编码器实现
 */

#include "DeltaEncoder.h"

#include <QDataStream>
#include <QIODevice>

// ═══════════════════════════════════════════════════════════
// 构造 / 析构
// ═══════════════════════════════════════════════════════════

DeltaEncoder::DeltaEncoder(QObject* parent) : QObject(parent) {}
DeltaEncoder::~DeltaEncoder() = default;

void DeltaEncoder::setMode(Mode mode) { m_mode = mode; }
DeltaEncoder::Mode DeltaEncoder::mode() const { return m_mode; }

// ═══════════════════════════════════════════════════════════
// ZigZag编解码
// ═══════════════════════════════════════════════════════════

quint64 DeltaEncoder::zigzagEncode(qint64 n)
{
    return static_cast<quint64>((n << 1) ^ (n >> 63));
}

qint64 DeltaEncoder::zigzagDecode(quint64 n)
{
    return static_cast<qint64>((n >> 1) ^ -(static_cast<qint64>(n) & 1));
}

// ═══════════════════════════════════════════════════════════
// 整数序列编解码
// ═══════════════════════════════════════════════════════════

QVector<qint64> DeltaEncoder::encodeInt64(const QVector<qint64>& values)
{
    if (values.isEmpty()) return {};

    QVector<qint64> result;
    result.reserve(values.size());

    result.append(values[0]); // 第一个值直接存储

    switch (m_mode) {
    case RawDelta:
        for (int i = 1; i < values.size(); ++i) {
            result.append(values[i] - values[i - 1]);
        }
        break;

    case ZigZagDelta:
        result[0] = static_cast<qint64>(zigzagEncode(values[0]));
        for (int i = 1; i < values.size(); ++i) {
            qint64 delta = values[i] - values[i - 1];
            result.append(static_cast<qint64>(zigzagEncode(delta)));
        }
        break;

    case XorDelta:
        result[0] = values[0];
        for (int i = 1; i < values.size(); ++i) {
            result.append(values[i] ^ values[i - 1]);
        }
        break;
    }

    m_stats.totalEncodes += 1;
    return result;
}

QVector<qint64> DeltaEncoder::decodeInt64(const QVector<qint64>& deltas)
{
    if (deltas.isEmpty()) return {};

    QVector<qint64> result;
    result.reserve(deltas.size());

    switch (m_mode) {
    case RawDelta:
        result.append(deltas[0]);
        for (int i = 1; i < deltas.size(); ++i) {
            result.append(result.last() + deltas[i]);
        }
        break;

    case ZigZagDelta:
        result.append(zigzagDecode(static_cast<quint64>(deltas[0])));
        for (int i = 1; i < deltas.size(); ++i) {
            qint64 delta = zigzagDecode(static_cast<quint64>(deltas[i]));
            result.append(result.last() + delta);
        }
        break;

    case XorDelta:
        result.append(deltas[0]);
        for (int i = 1; i < deltas.size(); ++i) {
            result.append(result.last() ^ deltas[i]);
        }
        break;
    }

    m_stats.totalDecodes += 1;
    return result;
}

// ═══════════════════════════════════════════════════════════
// 字节流编解码
// ═══════════════════════════════════════════════════════════

QByteArray DeltaEncoder::encode(const QByteArray& data)
{
    if (data.isEmpty()) {
        m_stats.totalEncodes += 1;
        emit encoded({}, 0.0);
        return {};
    }

    QByteArray result;
    result.reserve(data.size() + 4);

    // 头部: 原始长度(4字节) + 模式(1字节)
    result.append(static_cast<char>((data.size() >> 24) & 0xFF));
    result.append(static_cast<char>((data.size() >> 16) & 0xFF));
    result.append(static_cast<char>((data.size() >> 8) & 0xFF));
    result.append(static_cast<char>(data.size() & 0xFF));
    result.append(static_cast<char>(m_mode));

    // 第一个字节直接存储
    result.append(data[0]);

    switch (m_mode) {
    case RawDelta:
        for (int i = 1; i < data.size(); ++i) {
            auto delta = static_cast<quint8>(data[i]) -
                         static_cast<quint8>(data[i - 1]);
            result.append(static_cast<char>(delta));
        }
        break;

    case ZigZagDelta: {
        // 第一个字节用ZigZag编码
        auto first = static_cast<qint8>(data[0]);
        result[5] = static_cast<char>(zigzagEncode(first) & 0xFF);
        // 后续字节差分+ZigZag
        for (int i = 1; i < data.size(); ++i) {
            qint16 delta = static_cast<quint8>(data[i]) -
                           static_cast<quint8>(data[i - 1]);
            quint64 zz = zigzagEncode(delta);
            if (zz <= 0xFF) {
                result.append(static_cast<char>(zz));
            } else {
                // 多字节编码(简化: 2字节)
                result.append(static_cast<char>(0xFF));
                result.append(static_cast<char>(zz & 0xFF));
                result.append(static_cast<char>((zz >> 8) & 0xFF));
            }
        }
        break;
    }

    case XorDelta:
        for (int i = 1; i < data.size(); ++i) {
            result.append(static_cast<char>(
                static_cast<quint8>(data[i]) ^ static_cast<quint8>(data[i - 1])));
        }
        break;
    }

    m_stats.totalEncodes += 1;
    m_stats.totalBytesIn += static_cast<quint64>(data.size());
    m_stats.totalBytesOut += static_cast<quint64>(result.size());

    const double ratio = static_cast<double>(result.size()) /
                         static_cast<double>(std::max(data.size(), qsizetype(1)));
    if (m_stats.totalEncodes == 1) {
        m_stats.avgRatio = ratio;
    } else {
        m_stats.avgRatio += (ratio - m_stats.avgRatio) /
                            static_cast<double>(m_stats.totalEncodes);
    }

    emit encoded(result, ratio);
    return result;
}

QByteArray DeltaEncoder::decode(const QByteArray& data)
{
    if (data.size() < 6) {
        emit error(tr("Delta解码错误: 数据太短"));
        return {};
    }

    // 读取头部
    int originalSize = (static_cast<quint8>(data[0]) << 24) |
                       (static_cast<quint8>(data[1]) << 16) |
                       (static_cast<quint8>(data[2]) << 8) |
                       static_cast<quint8>(data[3]);
    auto modeUsed = static_cast<Mode>(static_cast<quint8>(data[4]));

    QByteArray result;
    result.reserve(originalSize);

    if (originalSize <= 0) {
        m_stats.totalDecodes += 1;
        emit decoded({});
        return {};
    }

    int pos = 5;
    quint8 prev = static_cast<quint8>(data[pos++]);
    result.append(static_cast<char>(prev));

    switch (modeUsed) {
    case RawDelta:
        while (result.size() < originalSize && pos < data.size()) {
            auto delta = static_cast<quint8>(data[pos++]);
            prev = prev + delta;
            result.append(static_cast<char>(prev));
        }
        break;

    case ZigZagDelta: {
        // 首字节ZigZag解码
        prev = static_cast<quint8>(zigzagDecode(
            static_cast<quint64>(static_cast<qint8>(data[5]))));
        result[0] = static_cast<char>(prev);
        pos = 6;
        while (result.size() < originalSize && pos < data.size()) {
            if (static_cast<quint8>(data[pos]) == 0xFF && pos + 2 < data.size()) {
                quint64 zz = static_cast<quint8>(data[pos + 1]) |
                             (static_cast<quint64>(static_cast<quint8>(data[pos + 2])) << 8);
                qint16 delta = static_cast<qint16>(zigzagDecode(zz));
                prev = static_cast<quint8>(static_cast<qint16>(prev) + delta);
                pos += 3;
            } else {
                qint16 delta = static_cast<qint16>(zigzagDecode(
                    static_cast<quint64>(data[pos])));
                prev = static_cast<quint8>(static_cast<qint16>(prev) + delta);
                pos++;
            }
            result.append(static_cast<char>(prev));
        }
        break;
    }

    case XorDelta:
        while (result.size() < originalSize && pos < data.size()) {
            prev = prev ^ static_cast<quint8>(data[pos++]);
            result.append(static_cast<char>(prev));
        }
        break;
    }

    m_stats.totalDecodes += 1;
    emit decoded(result);
    return result;
}

// ═══════════════════════════════════════════════════════════
// 统计
// ═══════════════════════════════════════════════════════════

DeltaEncoder::Stats DeltaEncoder::stats() const { return m_stats; }

void DeltaEncoder::resetStatistics() { m_stats = Stats{}; }
