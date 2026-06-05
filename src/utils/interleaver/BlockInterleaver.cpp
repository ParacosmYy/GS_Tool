/**
 * @file BlockInterleaver.cpp
 * @brief 块交织器实现
 */

#include "utils/interleaver/BlockInterleaver.h"

#include <QElapsedTimer>

BlockInterleaver::BlockInterleaver(QObject* parent)
    : QObject(parent), m_rows(4), m_cols(4), m_mode(Mode::RowColumn), m_timeSum(0.0) {}

void BlockInterleaver::setDimensions(int rows, int cols)
{
    m_rows = qMax(2, rows);
    m_cols = qMax(2, cols);
}

void BlockInterleaver::setMode(Mode mode) { m_mode = mode; }

QByteArray BlockInterleaver::interleave(const QByteArray& data)
{
    QElapsedTimer timer;
    timer.start();

    QByteArray result;
    switch (m_mode) {
    case Mode::RowColumn: result = rowColumnOp(data, false); break;
    case Mode::Diagonal:  result = diagonalOp(data, false); break;
    case Mode::Helical:   result = rowColumnOp(data, false); break;
    }

    m_stats.totalOperations++;
    m_stats.totalBlocksProcessed += data.size() / (m_rows * m_cols);
    m_timeSum += timer.elapsed();
    m_stats.averageProcessingTimeMs = m_timeSum / m_stats.totalOperations;

    emit operationCompleted(data.size());
    return result;
}

QByteArray BlockInterleaver::deinterleave(const QByteArray& data)
{
    QElapsedTimer timer;
    timer.start();

    QByteArray result;
    switch (m_mode) {
    case Mode::RowColumn: result = rowColumnOp(data, true); break;
    case Mode::Diagonal:  result = diagonalOp(data, true); break;
    case Mode::Helical:   result = rowColumnOp(data, true); break;
    }

    m_stats.totalOperations++;
    m_stats.totalBlocksProcessed += data.size() / (m_rows * m_cols);
    m_timeSum += timer.elapsed();
    m_stats.averageProcessingTimeMs = m_timeSum / m_stats.totalOperations;

    emit operationCompleted(data.size());
    return result;
}

QByteArray BlockInterleaver::rowColumnOp(const QByteArray& data, bool inverse)
{
    int blockSize = m_rows * m_cols;
    QByteArray result(data.size(), Qt::Uninitialized);

    for (int block = 0; block * blockSize < data.size(); ++block) {
        int offset = block * blockSize;
        for (int i = 0; i < blockSize && offset + i < data.size(); ++i) {
            int r = i / m_cols;
            int c = i % m_cols;
            int srcIdx, dstIdx;
            if (!inverse) {
                srcIdx = r * m_cols + c;
                dstIdx = c * m_rows + r;
            } else {
                srcIdx = c * m_rows + r;
                dstIdx = r * m_cols + c;
            }
            if (offset + srcIdx < data.size() && offset + dstIdx < data.size()) {
                result[offset + dstIdx] = data[offset + srcIdx];
            }
        }
    }
    /* 复制剩余字节 */
    int remaining = data.size() % blockSize;
    for (int i = data.size() - remaining; i < data.size(); ++i) {
        if (i >= 0 && i < result.size()) result[i] = data[i];
    }
    return result;
}

QByteArray BlockInterleaver::diagonalOp(const QByteArray& data, bool inverse)
{
    int blockSize = m_rows * m_cols;
    QByteArray result(data.size(), Qt::Uninitialized);

    for (int block = 0; block * blockSize < data.size(); ++block) {
        int offset = block * blockSize;
        int idx = 0;
        for (int d = 0; d < m_rows + m_cols - 1; ++d) {
            for (int r = 0; r < m_rows; ++r) {
                int c = d - r;
                if (c >= 0 && c < m_cols) {
                    int pos = r * m_cols + c;
                    if (offset + pos < data.size() && offset + idx < data.size()) {
                        if (!inverse) result[offset + idx] = data[offset + pos];
                        else result[offset + pos] = data[offset + idx];
                    }
                    ++idx;
                }
            }
        }
    }
    int remaining = data.size() % blockSize;
    for (int i = data.size() - remaining; i < data.size(); ++i) {
        if (i >= 0 && i < result.size()) result[i] = data[i];
    }
    return result;
}

void BlockInterleaver::resetStatistics() { m_stats = Stats{}; m_timeSum = 0.0; }
