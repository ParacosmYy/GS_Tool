/**
 * @file SplitRadixFFT6.cpp
 * @brief SplitRadixFFT6 实现
 *
 * 实现分裂基FFT：共轭对旋转因子预计算、迭代深度优先蝶形调度。
 */

#include "utils/fft220/SplitRadixFFT6.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

SplitRadixFFT6::SplitRadixFFT6(QObject *parent) : QObject(parent) {}
SplitRadixFFT6::~SplitRadixFFT6() = default;

/* ---- Bit reversal ---- */

int SplitRadixFFT6::bitReverse(int x, int log2n) const
{
    int rev = 0;
    for (int i = 0; i < log2n; ++i) {
        rev = (rev << 1) | (x & 1);
        x >>= 1;
    }
    return rev;
}

/* ---- Precompute twiddles ---- */

void SplitRadixFFT6::computeTwiddles()
{
    int halfN = m_size / 2;
    int quarterN = m_size / 4;

    // Full cos/sin tables for conjugate-pair access
    m_cosTable.resize(halfN);
    m_sinTable.resize(halfN);
    for (int k = 0; k < halfN; ++k) {
        double angle = 2.0 * M_PI * k / m_size;
        m_cosTable[k] = qCos(angle);
        m_sinTable[k] = qSin(angle);
    }
}

/* ---- Configuration ---- */

void SplitRadixFFT6::setParameters(int size)
{
    // Round up to next power of 2
    int n = 1;
    m_log2n = 0;
    while (n < size) { n <<= 1; m_log2n++; }
    m_size = qMax(1, n);
    m_stats.transformSize = m_size;
    computeTwiddles();
}

/* ---- Split-radix butterfly (iterative depth-first) ---- */

void SplitRadixFFT6::splitRadixButterfly(QVector<double>& re,
                                           QVector<double>& im,
                                           bool inverse) const
{
    int n = m_size;
    int sign = inverse ? 1 : -1;

    // Bit-reversal permutation
    for (int i = 0; i < n; ++i) {
        int j = bitReverse(i, m_log2n);
        if (j > i) {
            std::swap(re[i], re[j]);
            std::swap(im[i], im[j]);
        }
    }

    // Iterative depth-first butterfly stages
    // L-shaped butterflies: split N into one N/2 and two N/4 sub-transforms
    int butterflies = 0;

    // Size-2 and size-4 base butterflies
    for (int i = 0; i + 1 < n; i += 2) {
        double tRe = re[i] - re[i + 1];
        double tIm = im[i] - im[i + 1];
        re[i] += re[i + 1];
        im[i] += im[i + 1];
        re[i + 1] = tRe;
        im[i + 1] = tIm;
        butterflies++;
    }

    if (n >= 4) {
        for (int i = 0; i + 3 < n; i += 4) {
            double t0Re = re[i] - re[i + 2];
            double t0Im = im[i] - im[i + 2];
            re[i] += re[i + 2];
            im[i] += im[i + 2];

            double t1Re = re[i + 1] - re[i + 3];
            double t1Im = im[i + 1] - im[i + 3];
            re[i + 1] += re[i + 3];
            im[i + 1] += im[i + 3];

            // Apply twiddle for the odd part
            double wr = m_cosTable[n / 4];
            double wi = sign * m_sinTable[n / 4];
            double uRe = t1Re * wr - t1Im * wi;
            double uIm = t1Re * wi + t1Im * wr;

            re[i + 2] = t0Re - uIm;
            im[i + 2] = t0Im + uRe;
            re[i + 3] = t0Re + uIm;
            im[i + 3] = t0Im - uRe;
            butterflies++;
        }
    }

    // General split-radix stages for sizes >= 8
    for (int blockSize = 8; blockSize <= n; blockSize <<= 1) {
        int halfBlock = blockSize / 2;
        int quarterBlock = blockSize / 4;

        for (int base = 0; base < n; base += blockSize) {
            // N/2 sub-transform (even indices)
            for (int k = 0; k < halfBlock; k += 2) {
                int idx0 = base + k;
                int idx1 = base + k + halfBlock;
                double tRe = re[idx0] - re[idx1];
                double tIm = im[idx0] - im[idx1];
                re[idx0] += re[idx1];
                im[idx0] += im[idx1];
                re[idx1] = tRe;
                im[idx1] = tIm;
                butterflies++;
            }

            // Two N/4 sub-transforms (odd indices) with conjugate twiddle pairs
            for (int k = 0; k < quarterBlock; ++k) {
                int twIdx = k * (n / blockSize);
                double wr = m_cosTable[twIdx];
                double wi = sign * m_sinTable[twIdx];

                int idx1 = base + 2 * k + 1;
                int idx2 = idx1 + halfBlock;

                double x1Re = re[idx1], x1Im = im[idx1];
                double x2Re = re[idx2], x2Im = im[idx2];

                // Conjugate-pair: W^k and W^(N/2-k) share cos, sin differs by sign
                double t1Re = x1Re * wr - x1Im * wi;
                double t1Im = x1Re * wi + x1Im * wr;
                double t2Re = x2Re * wr + x2Im * wi;  // conjugate
                double t2Im = -x2Re * wi + x2Im * wr;

                re[idx1] = t1Re + t2Re;
                im[idx1] = t1Im + t2Im;
                re[idx2] = t1Re - t2Re;
                im[idx2] = t1Im - t2Im;
                butterflies++;
            }
        }
    }

    m_stats.butterflyCount = butterflies;
}

/* ---- Forward ---- */

QVector<double> SplitRadixFFT6::forward(const QVector<double>& realInput)
{
    QElapsedTimer timer;
    timer.start();

    QVector<double> re(m_size, 0.0), im(m_size, 0.0);
    for (int i = 0; i < qMin(realInput.size(), m_size); ++i)
        re[i] = realInput[i];

    splitRadixButterfly(re, im, false);

    QVector<double> result(m_size * 2);
    for (int i = 0; i < m_size; ++i) {
        result[i * 2] = re[i];
        result[i * 2 + 1] = im[i];
    }

    m_stats.totalTransforms++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalTransforms;
    emit transformCompleted(m_size, m_stats.butterflyCount, timer.elapsed());
    return result;
}

/* ---- Inverse ---- */

QVector<double> SplitRadixFFT6::inverse(const QVector<double>& complexInterleaved)
{
    QElapsedTimer timer;
    timer.start();

    QVector<double> re(m_size, 0.0), im(m_size, 0.0);
    for (int i = 0; i < qMin(complexInterleaved.size() / 2, m_size); ++i) {
        re[i] = complexInterleaved[i * 2];
        im[i] = complexInterleaved[i * 2 + 1];
    }

    splitRadixButterfly(re, im, true);
    for (int i = 0; i < m_size; ++i) re[i] /= m_size;

    m_stats.totalTransforms++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalTransforms;
    return re;
}

/* ---- Magnitude spectrum ---- */

QVector<double> SplitRadixFFT6::magnitudeSpectrum(const QVector<double>& realInput)
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

void SplitRadixFFT6::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_cosTable.clear();
    m_sinTable.clear();
    m_size = 0;
    m_log2n = 0;
}
