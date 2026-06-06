/**
 * @file RunLengthCode3.cpp
 * @brief RunLengthCode3 实现
 *
 * 实现游程编码：基本RLE与PackBits变体编解码、多通道支持。
 */

#include "utils/code171/RunLengthCode3.h"

#include <QElapsedTimer>

/* ---- Construction / Destruction ---- */

RunLengthCode3::RunLengthCode3(QObject *parent)
    : QObject(parent)
{
}

RunLengthCode3::~RunLengthCode3() = default;

/* ---- Configuration ---- */

void RunLengthCode3::setMode(Mode mode) { m_mode = mode; }
void RunLengthCode3::setMaxRunLength(int maxRun) { m_maxRun = qMax(2, maxRun); }

/* ---- PackBits Encode ---- */

QByteArray RunLengthCode3::encodePackBits(const QByteArray& input) const
{
    QByteArray out;
    int n = input.size();
    int i = 0;

    while (i < n) {
        /* Check for a run of identical bytes */
        int runLen = 1;
        while (i + runLen < n && input[i + runLen] == input[i] && runLen < m_maxRun)
            runLen++;

        if (runLen >= 3) {
            /* Run header: -count+1 (negative), then value */
            out.append(static_cast<char>(-(runLen - 1)));
            out.append(input[i]);
            i += runLen;
        } else {
            /* Literal sequence */
            int litStart = i;
            int litLen = 0;

            while (i < n && litLen < m_maxRun) {
                /* Peek ahead: if next 3 bytes are same, stop literal */
                if (i + 2 < n && input[i] == input[i + 1] && input[i] == input[i + 2])
                    break;
                i++;
                litLen++;
            }

            if (litLen == 0) {
                litLen = 1;
                i = litStart + 1;
            } else {
                i = litStart + litLen;
            }

            /* Literal header: count-1 (positive), then values */
            out.append(static_cast<char>(litLen - 1));
            for (int j = 0; j < litLen; ++j)
                out.append(input[litStart + j]);
        }
    }
    return out;
}

/* ---- PackBits Decode ---- */

QByteArray RunLengthCode3::decodePackBits(const QByteArray& encoded) const
{
    QByteArray out;
    int n = encoded.size();
    int i = 0;

    while (i < n) {
        qint8 header = static_cast<qint8>(encoded[i]);
        i++;

        if (header < 0) {
            /* Run: repeat next byte (-header+1) times */
            int count = -header + 1;
            if (i < n) {
                char val = encoded[i++];
                for (int j = 0; j < count; ++j)
                    out.append(val);
            }
        } else {
            /* Literal: copy (header+1) bytes */
            int count = header + 1;
            for (int j = 0; j < count && i < n; ++j)
                out.append(encoded[i++]);
        }
    }
    return out;
}

/* ---- Basic RLE Encode ---- */

QByteArray RunLengthCode3::encodeBasic(const QByteArray& input) const
{
    QByteArray out;
    int n = input.size();
    int i = 0;

    while (i < n) {
        char val = input[i];
        int count = 1;
        while (i + count < n && input[i + count] == val && count < 255)
            count++;

        /* Store: count byte + value byte */
        out.append(static_cast<char>(count));
        out.append(val);
        i += count;
    }
    return out;
}

/* ---- Basic RLE Decode ---- */

QByteArray RunLengthCode3::decodeBasic(const QByteArray& encoded) const
{
    QByteArray out;
    int n = encoded.size();
    int i = 0;

    while (i + 1 < n) {
        int count = static_cast<quint8>(encoded[i]);
        char val = encoded[i + 1];
        for (int j = 0; j < count; ++j)
            out.append(val);
        i += 2;
    }
    return out;
}

/* ---- Public encode / decode ---- */

QByteArray RunLengthCode3::encode(const QByteArray& input) const
{
    QElapsedTimer timer;
    timer.start();

    QByteArray result = (m_mode == PackBits)
        ? encodePackBits(input)
        : encodeBasic(input);

    m_stats.totalEncodes++;
    double ratio = compressionRatio(input, result);
    double sum = m_stats.avgCompressionRatio * (m_stats.totalEncodes - 1) + ratio;
    m_stats.avgCompressionRatio = sum / m_stats.totalEncodes;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = (m_stats.totalEncodes + m_stats.totalDecodes > 0)
        ? m_timeSum / (m_stats.totalEncodes + m_stats.totalDecodes) : 0.0;

    emit encodeCompleted(input.size(), result.size());
    return result;
}

QByteArray RunLengthCode3::decode(const QByteArray& encoded) const
{
    QElapsedTimer timer;
    timer.start();

    QByteArray result = (m_mode == PackBits)
        ? decodePackBits(encoded)
        : decodeBasic(encoded);

    m_stats.totalDecodes++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = (m_stats.totalEncodes + m_stats.totalDecodes > 0)
        ? m_timeSum / (m_stats.totalEncodes + m_stats.totalDecodes) : 0.0;

    emit decodeCompleted(result.size());
    return result;
}

/* ---- Multi-channel ---- */

QVector<QByteArray> RunLengthCode3::encodeMultiChannel(
    const QVector<QByteArray>& channels) const
{
    QVector<QByteArray> result;
    result.reserve(channels.size());
    for (const auto& ch : channels)
        result.append(encode(ch));
    return result;
}

QVector<QByteArray> RunLengthCode3::decodeMultiChannel(
    const QVector<QByteArray>& encoded) const
{
    QVector<QByteArray> result;
    result.reserve(encoded.size());
    for (const auto& enc : encoded)
        result.append(decode(enc));
    return result;
}

/* ---- Compression ratio ---- */

double RunLengthCode3::compressionRatio(const QByteArray& input,
                                         const QByteArray& encoded) const
{
    if (input.isEmpty()) return 1.0;
    return static_cast<double>(encoded.size()) / static_cast<double>(input.size());
}

/* ---- Statistics ---- */

void RunLengthCode3::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
