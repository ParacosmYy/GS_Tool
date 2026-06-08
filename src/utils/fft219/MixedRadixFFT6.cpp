/**
 * @file MixedRadixFFT6.cpp
 * @brief MixedRadixFFT6 实现
 *
 * 实现混合基数FFT：自动素因子分解、旋转因子预计算、任意复合长度变换。
 */

#include "utils/fft219/MixedRadixFFT6.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

MixedRadixFFT6::MixedRadixFFT6(QObject *parent) : QObject(parent) {}
MixedRadixFFT6::~MixedRadixFFT6() = default;

/* ---- Factorize N into prime factors ---- */

QVector<int> MixedRadixFFT6::factorize(int n) const
{
    QVector<int> factors;
    for (int d = 2; d * d <= n; ++d) {
        while (n % d == 0) { factors.append(d); n /= d; }
    }
    if (n > 1) factors.append(n);
    return factors;
}

/* ---- Set parameters ---- */

void MixedRadixFFT6::setParameters(int size)
{
    m_size = qMax(1, size);
    m_factors = factorize(m_size);
    m_stats.transformSize = m_size;
    m_stats.numFactors = m_factors.size();
    computePermutation();
    computeTwiddles();
}

/* ---- Compute index permutation ---- */

void MixedRadixFFT6::computePermutation()
{
    int n = m_size;
    m_perm.resize(n);
    int numFactors = m_factors.size();

    // Digit-reversal permutation based on mixed radices
    for (int i = 0; i < n; ++i) {
        int val = i, rev = 0;
        for (int f = 0; f < numFactors; ++f) {
            rev = rev * m_factors[f] + (val % m_factors[f]);
            val /= m_factors[f];
        }
        m_perm[i] = rev;
    }
}

/* ---- Precompute twiddle factors ---- */

void MixedRadixFFT6::computeTwiddles()
{
    m_twiddles.resize(m_size * 2);
    for (int k = 0; k < m_size; ++k) {
        double angle = -2.0 * M_PI * k / m_size;
        m_twiddles[k * 2] = qCos(angle);
        m_twiddles[k * 2 + 1] = qSin(angle);
    }
}

/* ---- Radix-p butterfly ---- */

void MixedRadixFFT6::radixButterfly(QVector<double>& re, QVector<double>& im,
                                      int radix, int base, int stride,
                                      int groupSize, bool inverse) const
{
    // Collect radix points
    QVector<double> pr(radix), pi(radix);

    for (int k = 0; k < groupSize; ++k) {
        // Gather
        for (int b = 0; b < radix; ++b) {
            int idx = base + k + b * stride * groupSize;
            // Apply twiddle
            int twIdx = (b * k * stride) % m_size;
            double wr = m_twiddles[twIdx * 2];
            double wi = m_twiddles[twIdx * 2 + 1];
            if (inverse) wi = -wi;
            pr[b] = re[idx] * wr - im[idx] * wi;
            pi[b] = re[idx] * wi + im[idx] * wr;
        }

        // DFT of length radix
        QVector<double> yr(radix, 0.0), yi(radix, 0.0);
        for (int j = 0; j < radix; ++j) {
            for (int b = 0; b < radix; ++b) {
                double angle = 2.0 * M_PI * j * b / radix;
                if (!inverse) angle = -angle;
                double w = qCos(angle);
                double s = qSin(angle);
                yr[j] += pr[b] * w - pi[b] * s;
                yi[j] += pr[b] * s + pi[b] * w;
            }
        }

        // Scatter
        for (int b = 0; b < radix; ++b) {
            int idx = base + k + b * stride * groupSize;
            re[idx] = yr[b];
            im[idx] = yi[b];
        }
    }
}

/* ---- Main transform ---- */

void MixedRadixFFT6::transform(QVector<double>& re, QVector<double>& im,
                                 bool inverse)
{
    // Apply permutation
    for (int i = 0; i < m_size; ++i) {
        if (m_perm[i] > i) {
            std::swap(re[i], re[m_perm[i]]);
            std::swap(im[i], im[m_perm[i]]);
        }
    }

    // Iterate through factors
    int stride = 1;
    for (int f = 0; f < m_factors.size(); ++f) {
        int radix = m_factors[f];
        int numGroups = m_size / (radix * stride);

        for (int g = 0; g < numGroups; ++g) {
            int base = g * radix * stride;
            radixButterfly(re, im, radix, base, stride, stride, inverse);
        }
        stride *= radix;
    }
}

/* ---- Forward ---- */

QVector<double> MixedRadixFFT6::forward(const QVector<double>& realInput)
{
    QElapsedTimer timer;
    timer.start();

    QVector<double> re(m_size, 0.0), im(m_size, 0.0);
    for (int i = 0; i < qMin(realInput.size(), m_size); ++i)
        re[i] = realInput[i];

    transform(re, im, false);

    QVector<double> result(m_size * 2);
    for (int i = 0; i < m_size; ++i) {
        result[i * 2] = re[i];
        result[i * 2 + 1] = im[i];
    }

    m_stats.totalTransforms++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalTransforms;
    emit transformCompleted(m_size, m_factors.size(), timer.elapsed());
    return result;
}

/* ---- Inverse ---- */

QVector<double> MixedRadixFFT6::inverse(const QVector<double>& complexInterleaved)
{
    QElapsedTimer timer;
    timer.start();

    QVector<double> re(m_size, 0.0), im(m_size, 0.0);
    for (int i = 0; i < qMin(complexInterleaved.size() / 2, m_size); ++i) {
        re[i] = complexInterleaved[i * 2];
        im[i] = complexInterleaved[i * 2 + 1];
    }

    transform(re, im, true);
    for (int i = 0; i < m_size; ++i) re[i] /= m_size;

    m_stats.totalTransforms++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalTransforms;
    return re;
}

/* ---- Factors ---- */

QVector<int> MixedRadixFFT6::factors() const { return m_factors; }

/* ---- Magnitude spectrum ---- */

QVector<double> MixedRadixFFT6::magnitudeSpectrum(
    const QVector<double>& realInput)
{
    QVector<double> spec = forward(realInput);
    int half = m_size / 2;
    QVector<double> mag(half);
    for (int i = 0; i < half; ++i) {
        double r = spec[i * 2], im = spec[i * 2 + 1];
        mag[i] = qSqrt(r * r + im * im);
    }
    return mag;
}

/* ---- Reset ---- */

void MixedRadixFFT6::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_twiddles.clear();
    m_perm.clear();
    m_factors.clear();
    m_size = 0;
}
