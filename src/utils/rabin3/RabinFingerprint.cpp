/**
 * @file RabinFingerprint.cpp
 * @brief Rabin指纹实现
 */

#include "RabinFingerprint.h"
#include <QElapsedTimer>

RabinFingerprint::RabinFingerprint(int windowSize, quint64 polynomial,
                                       QObject* parent)
    : QObject(parent)
    , m_windowSize(qMax(8, windowSize))
    , m_polynomial(polynomial)
    , m_fingerprint(0)
    , m_windowPos(0)
    , m_timeSum(0.0)
{
    m_window.fill('\0', m_windowSize);
    buildTable();
}

void RabinFingerprint::buildTable()
{
    int deg = 64;
    m_table.resize(256);
    m_outTable.resize(256);

    for (int i = 0; i < 256; ++i) {
        quint64 fp = i;
        for (int j = 0; j < 8; ++j) {
            if (fp & 1)
                fp = (fp >> 1) ^ m_polynomial;
            else
                fp >>= 1;
        }
        m_table[i] = fp;
    }

    quint64 outBase = 1;
    for (int j = 0; j < (m_windowSize * 8); ++j) {
        if (outBase & 1)
            outBase = (outBase >> 1) ^ m_polynomial;
        else
            outBase >>= 1;
    }

    for (int i = 0; i < 256; ++i) {
        quint64 fp = i;
        for (int j = 0; j < 8; ++j) {
            if (fp & 1)
                fp = (fp >> 1) ^ m_polynomial;
            else
                fp >>= 1;
        }
        m_outTable[i] = fp * outBase;
    }
}

quint64 RabinFingerprint::slide(quint8 byte)
{
    quint8 outByte = static_cast<quint8>(m_window[m_windowPos]);
    m_window[m_windowPos] = static_cast<char>(byte);
    m_windowPos = (m_windowPos + 1) % m_windowSize;

    m_fingerprint = (m_fingerprint << 8) ^ m_table[byte];
    m_fingerprint ^= m_outTable[outByte];

    m_stats.totalUpdates++;
    return m_fingerprint;
}

quint64 RabinFingerprint::fingerprint(const QByteArray& data)
{
    QElapsedTimer timer;
    timer.start();

    reset();
    for (int i = 0; i < data.size(); ++i)
        slide(static_cast<quint8>(data[i]));

    m_stats.totalBytes += data.size();
    m_timeSum += timer.elapsed();
    int total = m_stats.totalUpdates + m_stats.totalChunks;
    if (total > 0) m_stats.avgProcessingTimeMs = m_timeSum / total;

    return m_fingerprint;
}

QVector<int> RabinFingerprint::chunk(const QByteArray& data,
                                        int minChunk, int maxChunk,
                                        quint64 mask)
{
    QElapsedTimer timer;
    timer.start();

    QVector<int> boundaries;
    reset();

    int start = 0;
    for (int i = 0; i < data.size(); ++i) {
        slide(static_cast<quint8>(data[i]));
        int chunkLen = i - start + 1;

        if (chunkLen >= minChunk) {
            if ((m_fingerprint & mask) == 0 || chunkLen >= maxChunk) {
                boundaries.append(i + 1);
                start = i + 1;
                m_stats.totalChunks++;
            }
        }
    }

    if (start < data.size()) {
        boundaries.append(data.size());
        m_stats.totalChunks++;
    }

    m_stats.totalBytes += data.size();
    m_timeSum += timer.elapsed();
    int total = m_stats.totalUpdates + m_stats.totalChunks;
    if (total > 0) m_stats.avgProcessingTimeMs = m_timeSum / total;

    emit chunkingCompleted(boundaries.size());
    return boundaries;
}

void RabinFingerprint::reset()
{
    m_fingerprint = 0;
    m_window.fill('\0', m_windowSize);
    m_windowPos = 0;
}

quint64 RabinFingerprint::currentFingerprint() const { return m_fingerprint; }

RabinFingerprint::Stats RabinFingerprint::stats() const { return m_stats; }

void RabinFingerprint::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
