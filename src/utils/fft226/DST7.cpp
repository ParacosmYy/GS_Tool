/**
 * @file DST7.cpp
 * @brief DST7 实现
 *
 * 实现Type-IV DST：对称扩展、前预扭DCT管线快速计算。
 */

#include "utils/fft226/DST7.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

DST7::DST7(QObject *parent) : QObject(parent) {}
DST7::~DST7() = default;

/* ---- Prepare ---- */

bool DST7::prepare(int n)
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

    computePreTwiddles();
    computePostTwiddles();

    m_stats.numTwiddles = m_preTwiddleCos.size() + m_postTwiddleCos.size();
    return true;
}

/* ---- Compute pre-twiddle factors ---- */

void DST7::computePreTwiddles()
{
    // DST-IV: y[k] = sum_{n=0}^{N-1} x[n] * sin(pi*(n+0.5)*(k+0.5)/N)
    // Pre-twiddle for conversion to DCT-based pipeline
    int half = m_n / 2;
    m_preTwiddleCos.resize(half);
    m_preTwiddleSin.resize(half);

    for (int k = 0; k < half; ++k) {
        double angle = M_PI * (4 * k + 1) / (4.0 * m_n);
        m_preTwiddleCos[k] = qCos(angle);
        m_preTwiddleSin[k] = qSin(angle);
    }
}

/* ---- Compute post-twiddle factors ---- */

void DST7::computePostTwiddles()
{
    int half = m_n / 2;
    m_postTwiddleCos.resize(half);
    m_postTwiddleSin.resize(half);

    for (int k = 0; k < half; ++k) {
        // Post-twiddle for DST-IV output extraction
        double angle = M_PI * (2 * k + 1) / (4.0 * m_n);
        m_postTwiddleCos[k] = qCos(angle);
        m_postTwiddleSin[k] = qSin(angle);
    }
}

/* ---- Symmetric extension ---- */

QVector<double> DST7::symmetricExtend(const QVector<double>& input) const
{
    // DST-IV uses odd-symmetric extension: x[-n-1] = -x[n], x[2N+n] = -x[2N-1-n]
    int n = m_n;
    QVector<double> extended(2 * n);
    for (int i = 0; i < n; ++i) {
        extended[i] = input[i];
        extended[2 * n - 1 - i] = -input[i];
    }
    return extended;
}

/* ---- Bit reversal ---- */

void DST7::bitReverse(QVector<double>& data) const
{
    int n = data.size();
    for (int i = 1, j = 0; i < n; ++i) {
        int bit = n >> 1;
        while (j & bit) { j ^= bit; bit >>= 1; }
        j ^= bit;
        if (i < j) std::swap(data[i], data[j]);
    }
}

/* ---- DCT-II butterfly ---- */

void DST7::dct2Butterfly(QVector<double>& data) const
{
    int n = data.size();
    for (int len = 2; len <= n; len <<= 1) {
        double angle = -M_PI / len;
        for (int i = 0; i < n; i += len) {
            for (int j = 0; j < len / 2; ++j) {
                double wRe = qCos(angle * (j + 0.5));
                double wIm = qSin(angle * (j + 0.5));
                double a = data[i + j];
                double b = data[i + j + len / 2];
                data[i + j] = a + b;
                data[i + j + len / 2] = (a - b) * wRe;
            }
        }
    }
}

/* ---- Apply pre-twiddle ---- */

void DST7::applyPreTwiddle(QVector<double>& data) const
{
    int half = m_n / 2;
    for (int k = 0; k < half; ++k) {
        double re = data[k] * m_preTwiddleCos[k] - data[half + k] * m_preTwiddleSin[k];
        double im = data[k] * m_preTwiddleSin[k] + data[half + k] * m_preTwiddleCos[k];
        data[k] = re;
        data[half + k] = im;
    }
}

/* ---- Apply post-twiddle ---- */

void DST7::applyPostTwiddle(QVector<double>& data) const
{
    int half = m_n / 2;
    for (int k = 0; k < half; ++k) {
        double re = data[k] * m_postTwiddleCos[k] - data[half + k] * m_postTwiddleSin[k];
        double im = data[k] * m_postTwiddleSin[k] + data[half + k] * m_postTwiddleCos[k];
        data[k] = re;
        data[half + k] = im;
    }
}

/* ---- Forward DST-IV ---- */

QVector<double> DST7::forward(const QVector<double>& input) const
{
    QElapsedTimer timer;
    timer.start();

    int n = m_n;
    QVector<double> data(n, 0.0);
    for (int i = 0; i < qMin(input.size(), n); ++i)
        data[i] = input[i];

    // Step 1: Pre-processing - reorder and sign flip for DST->DCT conversion
    QVector<double> reordered(n);
    for (int k = 0; k < n / 2; ++k) {
        reordered[k] = data[2 * k];
        reordered[n - 1 - k] = data[2 * k + 1];
    }
    data = reordered;

    // Step 2: Apply pre-twiddle
    const_cast<DST7*>(this)->applyPreTwiddle(data);

    // Step 3: DCT-II butterfly stages
    for (int s = 0; s < m_stages; ++s) {
        int blockSize = 1 << (s + 1);
        int numBlocks = n / blockSize;
        for (int b = 0; b < numBlocks; ++b) {
            int base = b * blockSize;
            int half = blockSize / 2;
            for (int k = 0; k < half; ++k) {
                double angle = M_PI * (2 * k + 1) / (2.0 * blockSize);
                double cosA = qCos(angle);
                double sum = data[base + k] + data[base + half + k];
                double diff = (data[base + k] - data[base + half + k]) * cosA;
                data[base + k] = sum;
                data[base + half + k] = diff;
            }
        }
    }

    // Step 4: Apply post-twiddle
    const_cast<DST7*>(this)->applyPostTwiddle(data);

    // Step 5: Bit-reverse output
    const_cast<DST7*>(this)->bitReverse(data);

    // Normalization
    double scale = qSqrt(2.0 / n);
    for (int i = 0; i < n; ++i)
        data[i] *= scale;

    const_cast<DST7*>(this)->m_stats.totalOps++;
    const_cast<DST7*>(this)->m_timeSum += timer.elapsed();
    const_cast<DST7*>(this)->m_stats.avgProcessingTimeMs =
        m_timeSum / m_stats.totalOps;
    emit const_cast<DST7*>(this)->transformCompleted(n, timer.elapsed());
    return data;
}

/* ---- Inverse DST-IV ---- */

QVector<double> DST7::inverse(const QVector<double>& input) const
{
    // DST-IV is self-inverse: forward = inverse up to scale factor
    QVector<double> result = forward(input);
    double scale = 1.0 / m_n;
    for (int i = 0; i < result.size(); ++i)
        result[i] *= scale * m_n;
    return result;
}

/* ---- Reset ---- */

void DST7::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_preTwiddleCos.clear();
    m_preTwiddleSin.clear();
    m_postTwiddleCos.clear();
    m_postTwiddleSin.clear();
    m_n = 0;
    m_stages = 0;
}
