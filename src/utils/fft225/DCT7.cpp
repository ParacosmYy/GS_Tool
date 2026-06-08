/**
 * @file DCT7.cpp
 * @brief DCT7 实现
 *
 * 实现Type-IV DCT：正交因式分解、前后加法蝶形网络。
 */

#include "utils/fft225/DCT7.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

DCT7::DCT7(QObject *parent) : QObject(parent) {}
DCT7::~DCT7() = default;

/* ---- Prepare ---- */

bool DCT7::prepare(int n)
{
    if (n < 2) return false;
    int test = n;
    while (test > 1) {
        if (test % 2 != 0) return false;
        test /= 2;
    }

    m_n = n;
    m_stages = 0;
    int tmp = n;
    while (tmp > 1) { tmp /= 2; m_stages++; }

    m_stats.transformSize = n;
    m_stats.numStages = m_stages;

    // Compute twiddle factors for DCT-IV: cos(pi*(k+0.5)*(n+0.5)/n)
    // Using orthogonal factorization: twiddle = cos(pi*(4*k+1)/(4*n)) and sin
    m_twiddleCos.resize(n / 2);
    m_twiddleSin.resize(n / 2);
    for (int k = 0; k < n / 2; ++k) {
        double angle = M_PI * (4 * k + 1) / (4.0 * n);
        m_twiddleCos[k] = qCos(angle);
        m_twiddleSin[k] = qSin(angle);
    }

    // Build butterfly networks
    buildPreAddButterfly();
    buildPostAddButterfly();

    m_stats.numButterflies = n * m_stages;
    return true;
}

/* ---- Build pre-addition butterfly ---- */

void DCT7::buildPreAddButterfly()
{
    m_preAddIndices.clear();
    int half = m_n / 2;

    // Stage 0: reorder input with pre-addition
    // x[k] + x[N-1-k] and x[k] - x[N-1-k] pairs
    QVector<int> stage0(half * 2);
    for (int k = 0; k < half; ++k) {
        stage0[2 * k] = k;           // index for x[k]
        stage0[2 * k + 1] = m_n - 1 - k; // index for x[N-1-k]
    }
    m_preAddIndices.append(stage0);

    // Additional pre-add stages from log2 decomposition
    for (int s = 1; s < m_stages; ++s) {
        int blockSize = 1 << (s + 1);
        int numBlocks = m_n / blockSize;
        QVector<int> stage;
        for (int b = 0; b < numBlocks; ++b) {
            int base = b * blockSize;
            int halfBlock = blockSize / 2;
            for (int k = 0; k < halfBlock; ++k) {
                stage.append(base + k);
                stage.append(base + blockSize - 1 - k);
            }
        }
        m_preAddIndices.append(stage);
    }
}

/* ---- Build post-addition butterfly ---- */

void DCT7::buildPostAddButterfly()
{
    m_postAddIndices.clear();

    // Post-add stages: reverse of pre-add
    for (int s = m_stages - 1; s >= 0; --s) {
        int blockSize = 1 << (s + 1);
        int numBlocks = m_n / blockSize;
        QVector<int> stage;
        for (int b = 0; b < numBlocks; ++b) {
            int base = b * blockSize;
            int halfBlock = blockSize / 2;
            for (int k = 0; k < halfBlock; ++k) {
                stage.append(base + k);
                stage.append(base + halfBlock + k);
            }
        }
        m_postAddIndices.append(stage);
    }
}

/* ---- Apply pre-addition butterfly ---- */

void DCT7::applyPreAdd(QVector<double>& data) const
{
    int half = m_n / 2;
    QVector<double> temp(m_n);

    // First stage: compute sum/difference pairs
    for (int k = 0; k < half; ++k) {
        temp[k] = data[k] + data[m_n - 1 - k];
        temp[half + k] = data[k] - data[m_n - 1 - k];
    }
    data = temp;

    // Subsequent pre-add stages
    for (int s = 1; s < m_preAddIndices.size() && s < m_stages; ++s) {
        const QVector<int>& stage = m_preAddIndices[s];
        int numPairs = stage.size() / 2;
        QVector<double> buf(m_n);
        for (int p = 0; p < numPairs; ++p) {
            int a = stage[2 * p];
            int b = stage[2 * p + 1];
            if (a < m_n && b < m_n) {
                buf[a] = data[a] + data[b];
                buf[b] = data[a] - data[b];
            }
        }
        // Copy unprocessed elements
        for (int i = 0; i < m_n; ++i)
            if (buf[i] != 0.0 || data[i] == 0.0) data[i] = buf[i];
    }
}

/* ---- Apply twiddle factors ---- */

void DCT7::applyTwiddles(QVector<double>& data) const
{
    int half = m_n / 2;
    for (int k = 0; k < half; ++k) {
        double re = data[k] * m_twiddleCos[k] - data[half + k] * m_twiddleSin[k];
        double im = data[k] * m_twiddleSin[k] + data[half + k] * m_twiddleCos[k];
        data[k] = re;
        data[half + k] = im;
    }
}

/* ---- Apply post-addition butterfly ---- */

void DCT7::applyPostAdd(QVector<double>& data) const
{
    for (int s = 0; s < m_postAddIndices.size(); ++s) {
        const QVector<int>& stage = m_postAddIndices[s];
        int numPairs = stage.size() / 2;
        for (int p = 0; p < numPairs; ++p) {
            int a = stage[2 * p];
            int b = stage[2 * p + 1];
            if (a < m_n && b < m_n) {
                double sum = data[a] + data[b];
                double diff = data[a] - data[b];
                data[a] = sum;
                data[b] = diff;
            }
        }
    }
}

/* ---- Forward DCT-IV ---- */

QVector<double> DCT7::forward(const QVector<double>& input) const
{
    QElapsedTimer timer;
    timer.start();

    int n = m_n;
    QVector<double> data(n, 0.0);
    for (int i = 0; i < qMin(input.size(), n); ++i)
        data[i] = input[i];

    // Step 1: Pre-addition butterfly
    const_cast<DCT7*>(this)->applyPreAdd(data);

    // Step 2: Apply twiddle factors (orthogonal factorization)
    const_cast<DCT7*>(this)->applyTwiddles(data);

    // Step 3: Recursive half-size DCT-IV (simplified via DCT-II recursion)
    // Apply DCT-II style butterfly on each half
    int half = n / 2;
    for (int k = 0; k < half; ++k) {
        double angle = M_PI * k / n;
        double cosA = qCos(angle);
        double sinA = qSin(angle);
        double re = data[k] * cosA - data[half + k] * sinA;
        double im = data[k] * sinA + data[half + k] * cosA;
        data[k] = re;
        data[half + k] = im;
    }

    // Step 4: Post-addition butterfly
    const_cast<DCT7*>(this)->applyPostAdd(data);

    // Normalization for orthogonal DCT-IV
    double scale = qSqrt(2.0 / n);
    for (int i = 0; i < n; ++i)
        data[i] *= scale;

    const_cast<DCT7*>(this)->m_stats.totalOps++;
    const_cast<DCT7*>(this)->m_timeSum += timer.elapsed();
    const_cast<DCT7*>(this)->m_stats.avgProcessingTimeMs =
        m_timeSum / m_stats.totalOps;
    emit const_cast<DCT7*>(this)->transformCompleted(n, timer.elapsed());
    return data;
}

/* ---- Inverse DCT-IV ---- */

QVector<double> DCT7::inverse(const QVector<double>& input) const
{
    // DCT-IV is self-inverse: Y = DCT4(DCT4(x)) * N
    QVector<double> result = forward(input);
    double scale = 1.0 / m_n;
    for (int i = 0; i < result.size(); ++i)
        result[i] *= scale * m_n;
    return result;
}

/* ---- Reset ---- */

void DCT7::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_twiddleCos.clear();
    m_twiddleSin.clear();
    m_preAddIndices.clear();
    m_postAddIndices.clear();
    m_n = 0;
    m_stages = 0;
}
