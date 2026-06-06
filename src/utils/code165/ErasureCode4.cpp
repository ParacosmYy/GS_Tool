/**
 * @file ErasureCode4.cpp
 * @brief ErasureCode4 实现
 *
 * 实现Reed-Solomon纠删码：GF(256)有限域运算、Vandermonde矩阵编码、
 * 高斯消元解码恢复丢失分片。
 */

#include "utils/code165/ErasureCode4.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- GF(256) static tables ---- */
QVector<quint8> ErasureCode4::s_expTable;
QVector<quint8> ErasureCode4::s_logTable;
bool ErasureCode4::s_tablesInit = false;

void ErasureCode4::initGfTables()
{
    if (s_tablesInit) return;
    s_tablesInit = true;

    s_expTable.resize(512);
    s_logTable.resize(256);

    /* Generator polynomial: x^8 + x^4 + x^3 + x^2 + 1 = 0x11D */
    quint32 x = 1;
    for (int i = 0; i < 255; ++i) {
        s_expTable[i] = static_cast<quint8>(x);
        s_logTable[x] = static_cast<quint8>(i);
        x <<= 1;
        if (x & 0x100) x ^= 0x11D;
    }
    for (int i = 255; i < 512; ++i)
        s_expTable[i] = s_expTable[i - 255];
}

int ErasureCode4::gfLog(quint8 a)
{
    return (a == 0) ? 0 : s_logTable[a];
}

quint8 ErasureCode4::gfExp(int e)
{
    return s_expTable[e % 255];
}

quint8 ErasureCode4::gfMul(quint8 a, quint8 b)
{
    if (a == 0 || b == 0) return 0;
    return gfExp(gfLog(a) + gfLog(b));
}

quint8 ErasureCode4::gfInv(quint8 a)
{
    if (a == 0) return 0;
    return gfExp(255 - gfLog(a));
}

quint8 ErasureCode4::gfDiv(quint8 a, quint8 b)
{
    if (b == 0) return 0;
    if (a == 0) return 0;
    return gfExp(gfLog(a) + 255 - gfLog(b));
}

/* ---- ErasureCode4 ---- */

ErasureCode4::ErasureCode4(QObject* parent)
    : QObject(parent)
{
    initGfTables();
}

ErasureCode4::~ErasureCode4() = default;

void ErasureCode4::init(int dataShards, int parityShards)
{
    m_dataShards = qMax(1, dataShards);
    m_parityShards = qMax(1, parityShards);
    buildMatrix();
}

void ErasureCode4::buildMatrix()
{
    const int total = m_dataShards + m_parityShards;
    /* Vandermonde matrix: row i, col j = (i+1)^j in GF(256) */
    m_matrix.resize(total);
    for (int i = 0; i < total; ++i) {
        m_matrix[i].resize(m_dataShards);
        for (int j = 0; j < m_dataShards; ++j) {
            quint8 val = 1;
            for (int p = 0; p < j; ++p)
                val = gfMul(val, static_cast<quint8>(i + 1));
            m_matrix[i][j] = val;
        }
    }
}

QVector<QVector<quint8>> ErasureCode4::encode(const QVector<QVector<quint8>>& data)
{
    QElapsedTimer timer;
    timer.start();

    QVector<QVector<quint8>> parity(m_parityShards);
    if (data.size() < m_dataShards) return parity;

    int blockSize = data[0].size();
    for (int p = 0; p < m_parityShards; ++p) {
        parity[p].resize(blockSize, 0);
    }

    /* Matrix multiply: parity = matrix[dataShards..total-1] * data */
    for (int byteIdx = 0; byteIdx < blockSize; ++byteIdx) {
        for (int p = 0; p < m_parityShards; ++p) {
            quint8 sum = 0;
            for (int d = 0; d < m_dataShards; ++d) {
                quint8 val = (byteIdx < data[d].size()) ? data[d][byteIdx] : 0;
                sum = gfMul(m_matrix[m_dataShards + p][d], val) ^ sum;
            }
            parity[p][byteIdx] = sum;
        }
    }

    m_stats.totalEncodes++;
    m_stats.lastDataShards = m_dataShards;
    m_stats.lastParityShards = m_parityShards;
    m_timeSum += timer.elapsed();
    quint64 total = m_stats.totalEncodes + m_stats.totalDecodes;
    m_stats.avgProcessingTimeMs = (total > 0) ? m_timeSum / total : 0.0;

    emit encodeCompleted(m_dataShards + m_parityShards);
    return parity;
}

QVector<QVector<quint8>> ErasureCode4::decode(const QVector<QVector<quint8>>& shards,
                                               const QVector<int>& missing)
{
    QElapsedTimer timer;
    timer.start();

    int total = m_dataShards + m_parityShards;
    QVector<QVector<quint8>> result(m_dataShards);

    if (missing.isEmpty() || missing.size() > m_parityShards) {
        /* Nothing to recover or too many erasures */
        for (int i = 0; i < m_dataShards && i < shards.size(); ++i)
            result[i] = shards[i];
        return result;
    }

    /* Build decoding matrix from surviving rows */
    QVector<int> surviving;
    for (int i = 0; i < total; ++i) {
        if (!missing.contains(i) && surviving.size() < m_dataShards)
            surviving.append(i);
    }

    /* Extract sub-matrix (surviving rows) */
    int n = m_dataShards;
    QVector<QVector<quint8>> decMatrix(n);
    for (int i = 0; i < n; ++i) {
        decMatrix[i].resize(n);
        for (int j = 0; j < n; ++j)
            decMatrix[i][j] = m_matrix[surviving[i]][j];
    }

    /* Gaussian elimination to invert decMatrix */
    /* Augment with identity for inversion */
    QVector<QVector<quint8>> aug(n);
    for (int i = 0; i < n; ++i) {
        aug[i].resize(2 * n, 0);
        for (int j = 0; j < n; ++j) aug[i][j] = decMatrix[i][j];
        aug[i][n + i] = 1;
    }

    /* Forward elimination */
    for (int col = 0; col < n; ++col) {
        /* Find pivot */
        int pivot = -1;
        for (int row = col; row < n; ++row) {
            if (aug[row][col] != 0) { pivot = row; break; }
        }
        if (pivot < 0) continue;

        /* Swap rows */
        if (pivot != col) {
            for (int j = 0; j < 2 * n; ++j)
                std::swap(aug[col][j], aug[pivot][j]);
        }

        /* Scale pivot row */
        quint8 pivotVal = aug[col][col];
        quint8 invPivot = gfInv(pivotVal);
        for (int j = 0; j < 2 * n; ++j)
            aug[col][j] = gfMul(aug[col][j], invPivot);

        /* Eliminate column */
        for (int row = 0; row < n; ++row) {
            if (row == col || aug[row][col] == 0) continue;
            quint8 factor = aug[row][col];
            for (int j = 0; j < 2 * n; ++j)
                aug[row][j] ^= gfMul(factor, aug[col][j]);
        }
    }

    /* Extract inverse from right half */
    QVector<QVector<quint8>> invMatrix(n);
    for (int i = 0; i < n; ++i) {
        invMatrix[i].resize(n);
        for (int j = 0; j < n; ++j)
            invMatrix[i][j] = aug[i][n + j];
    }

    /* Recover missing data shards */
    int blockSize = 0;
    for (int i : surviving) {
        if (i < shards.size() && shards[i].size() > blockSize)
            blockSize = shards[i].size();
    }

    for (int d = 0; d < m_dataShards; ++d) {
        result[d].resize(blockSize, 0);
    }

    for (int byteIdx = 0; byteIdx < blockSize; ++byteIdx) {
        /* Gather surviving shard bytes */
        QVector<quint8> survBytes(n);
        for (int i = 0; i < n; ++i) {
            int si = surviving[i];
            survBytes[i] = (si < shards.size() && byteIdx < shards[si].size())
                ? shards[si][byteIdx] : 0;
        }

        /* Multiply inverse matrix by surviving bytes to recover all data */
        for (int d = 0; d < n; ++d) {
            quint8 sum = 0;
            for (int j = 0; j < n; ++j)
                sum ^= gfMul(invMatrix[d][j], survBytes[j]);
            result[d][byteIdx] = sum;
        }
    }

    m_stats.totalDecodes++;
    m_timeSum += timer.elapsed();
    quint64 tot = m_stats.totalEncodes + m_stats.totalDecodes;
    m_stats.avgProcessingTimeMs = (tot > 0) ? m_timeSum / tot : 0.0;

    emit decodeCompleted(missing.size());
    return result;
}

void ErasureCode4::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
