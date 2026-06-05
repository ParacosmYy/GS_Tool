/**
 * @file RunLengthCodec.cpp
 * @brief 行程编码器实现
 */

#include "utils/rle/RunLengthCodec.h"
#include <QElapsedTimer>

RunLengthCodec::RunLengthCodec(QObject* parent)
    : QObject(parent), m_minRunLength(4), m_markerByte(0xFF), m_timeSum(0.0) {}

void RunLengthCodec::setMinRunLength(int minRun) { m_minRunLength = qMax(2, minRun); }
void RunLengthCodec::setMarkerByte(quint8 marker) { m_markerByte = marker; }

QByteArray RunLengthCodec::encode(const QByteArray& data)
{
    QElapsedTimer timer;
    timer.start();

    QByteArray result;
    if (data.isEmpty()) return result;

    int i = 0;
    while (i < data.size()) {
        quint8 currentByte = static_cast<quint8>(data[i]);
        int runLength = 1;

        while (i + runLength < data.size() &&
               static_cast<quint8>(data[i + runLength]) == currentByte &&
               runLength < 255) {
            ++runLength;
        }

        if (runLength >= m_minRunLength) {
            result.append(static_cast<char>(m_markerByte));
            result.append(static_cast<char>(runLength));
            result.append(static_cast<char>(currentByte));
            i += runLength;
        } else {
            /* 检查是否需要转义标记字节 */
            if (currentByte == m_markerByte) {
                result.append(static_cast<char>(m_markerByte));
                result.append(static_cast<char>(1));
                result.append(static_cast<char>(currentByte));
            } else {
                result.append(data[i]);
            }
            ++i;
        }
    }

    double elapsed = timer.elapsed();
    ++m_stats.totalEncodes;
    m_stats.totalBytesIn += data.size();
    m_stats.totalBytesOut += result.size();
    m_stats.compressionRatio = (m_stats.totalBytesIn > 0)
        ? static_cast<double>(m_stats.totalBytesOut) / m_stats.totalBytesIn : 0.0;
    m_timeSum += elapsed;
    m_stats.averageProcessingTimeMs = m_timeSum / (m_stats.totalEncodes + m_stats.totalDecodes);

    emit encodeComplete(data.size(), result.size(), m_stats.compressionRatio);
    return result;
}

QByteArray RunLengthCodec::decode(const QByteArray& data)
{
    QElapsedTimer timer;
    timer.start();

    QByteArray result;
    if (data.isEmpty()) return result;

    int i = 0;
    while (i < data.size()) {
        if (static_cast<quint8>(data[i]) == m_markerByte && i + 2 < data.size()) {
            int count = static_cast<quint8>(data[i + 1]);
            char byte = data[i + 2];
            for (int j = 0; j < count; ++j) result.append(byte);
            i += 3;
        } else {
            result.append(data[i]);
            ++i;
        }
    }

    double elapsed = timer.elapsed();
    ++m_stats.totalDecodes;
    m_timeSum += elapsed;
    quint64 totalOps = m_stats.totalEncodes + m_stats.totalDecodes;
    m_stats.averageProcessingTimeMs = (totalOps > 0) ? m_timeSum / totalOps : 0.0;

    emit decodeComplete(data.size(), result.size());
    return result;
}

void RunLengthCodec::resetStatistics() { m_stats = Stats{}; m_timeSum = 0.0; }
