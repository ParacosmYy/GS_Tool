/**
 * @file WHT7.cpp
 * @brief WHT7 实现
 *
 * 实现沃尔什-哈达玛变换：序率排序快速计算与Gray码比特逆序重索引。
 */

#include "utils/fft241/WHT7.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

WHT7::WHT7(QObject *parent) : QObject(parent) {}
WHT7::~WHT7() = default;

/* ---- Helper functions ---- */

bool WHT7::isPowerOf2(int n) { return n > 0 && (n & (n - 1)) == 0; }

int WHT7::grayCode(int i) { return i ^ (i >> 1); }

int WHT7::bitReverse(int val, int log2n)
{
    int rev = 0;
    for (int b = 0; b < log2n; ++b) {
        rev = (rev << 1) | (val & 1);
        val >>= 1;
    }
    return rev;
}

/* ---- Configure ---- */

bool WHT7::configure(int N)
{
    if (N < 2 || !isPowerOf2(N)) return false;
    m_N = N;
    m_log2N = 0;
    int tmp = N;
    while (tmp > 1) { tmp >>= 1; m_log2N++; }
    m_stats.transformSize = N;
    return true;
}

/* ---- Build sequency reindex table via Gray code ---- */

QVector<int> WHT7::buildSequencyIndex() const
{
    int N = m_N;
    QVector<int> index(N);
    for (int i = 0; i < N; ++i) {
        int g = grayCode(i);
        index[i] = bitReverse(g, m_log2N);
    }
    return index;
}

/* ---- Natural-order fast WHT butterfly ---- */

void WHT7::fastWHT(QVector<double>& data) const
{
    int N = data.size();
    for (int len = 2; len <= N; len *= 2) {
        int half = len / 2;
        for (int i = 0; i < N; i += len) {
            for (int j = 0; j < half; ++j) {
                double a = data[i + j];
                double b = data[i + j + half];
                data[i + j] = a + b;
                data[i + j + half] = a - b;
            }
        }
    }
}

/* ---- Forward WHT (sequency-ordered) ---- */

QVector<double> WHT7::forward(const QVector<double>& input)
{
    if (input.size() != m_N) return {};
    QVector<double> data = input;
    forwardInPlace(data);
    return data;
}

/* ---- Inverse WHT (sequency-ordered) ---- */

QVector<double> WHT7::inverse(const QVector<double>& spectrum)
{
    if (spectrum.size() != m_N) return {};
    QVector<double> data = spectrum;
    inverseInPlace(data);
    return data;
}

/* ---- Forward in-place ---- */

void WHT7::forwardInPlace(QVector<double>& data)
{
    QElapsedTimer timer;
    timer.start();

    // Step 1: Bit-reversal permutation
    int N = data.size();
    QVector<int> brIndex(N);
    for (int i = 0; i < N; ++i)
        brIndex[i] = bitReverse(i, m_log2N);

    QVector<double> tmp(N);
    for (int i = 0; i < N; ++i)
        tmp[i] = data[brIndex[i]];
    data = tmp;

    // Step 2: Natural-order fast WHT
    fastWHT(data);

    // Step 3: Reorder to sequency (Walsh) order via Gray code
    QVector<int> seqIdx = buildSequencyIndex();
    tmp.resize(N);
    for (int i = 0; i < N; ++i)
        tmp[i] = data[seqIdx[i]];
    data = tmp;

    // Normalize by 1/sqrt(N) for orthonormal transform
    double scale = 1.0 / qSqrt(static_cast<double>(N));
    for (int i = 0; i < N; ++i)
        data[i] *= scale;

    m_stats.numForward++;
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit forwardCompleted(m_N, timer.elapsed());
}

/* ---- Inverse in-place ---- */

void WHT7::inverseInPlace(QVector<double>& data)
{
    QElapsedTimer timer;
    timer.start();

    // WHT is its own inverse (orthonormal), so inverse = transpose = forward
    int N = data.size();

    // Undo normalization
    double scale = qSqrt(static_cast<double>(N));
    for (int i = 0; i < N; ++i)
        data[i] *= scale;

    // Reverse sequency reordering: build inverse mapping
    QVector<int> seqIdx = buildSequencyIndex();
    QVector<double> tmp(N);
    for (int i = 0; i < N; ++i)
        tmp[seqIdx[i]] = data[i];
    data = tmp;

    // Natural-order fast WHT
    fastWHT(data);

    // Undo bit-reversal
    QVector<int> brInv(N);
    for (int i = 0; i < N; ++i)
        brInv[i] = bitReverse(i, m_log2N);
    tmp.resize(N);
    for (int i = 0; i < N; ++i)
        tmp[brInv[i]] = data[i];
    data = tmp;

    // Re-normalize
    scale = 1.0 / qSqrt(static_cast<double>(N));
    for (int i = 0; i < N; ++i)
        data[i] *= scale;

    m_stats.numInverse++;
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit inverseCompleted(m_N, timer.elapsed());
}

/* ---- Reset ---- */

void WHT7::resetStatistics()
{
    m_N = 0; m_log2N = 0;
    m_stats = Stats{}; m_timeSum = 0.0;
}
