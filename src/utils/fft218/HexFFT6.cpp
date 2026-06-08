/**
 * @file HexFFT6.cpp
 * @brief HexFFT6 实现
 *
 * 实现六角FFT：6点蝶形核、radix-6 Cooley-Tukey分解、数字反转置换。
 */

#include "utils/fft218/HexFFT6.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

HexFFT6::HexFFT6(QObject *parent) : QObject(parent)
{
    setParameters(216);
}

HexFFT6::~HexFFT6() = default;

/* ---- Configuration ---- */

void HexFFT6::setParameters(int size)
{
    // Find nearest power of 6 >= size
    int n = 1;
    m_stages = 0;
    while (n < size) { n *= 6; m_stages++; }
    m_size = qMax(6, n);

    m_stats.transformSize = m_size;
    m_stats.stages = m_stages;
    computeTwiddles();
}

/* ---- Precompute twiddle factors ---- */

void HexFFT6::computeTwiddles()
{
    // Twiddle factors for each stage: W_N^k = exp(-2*pi*k/N)
    m_twiddles.resize(m_size * 2);
    for (int k = 0; k < m_size; ++k) {
        double angle = -2.0 * M_PI * k / m_size;
        m_twiddles[k * 2] = qCos(angle);
        m_twiddles[k * 2 + 1] = qSin(angle);
    }
}

/* ---- 6-point DFT butterfly kernel ---- */

void HexFFT6::butterfly6(double& r0, double& i0, double& r1, double& i1,
                           double& r2, double& i2, double& r3, double& i3,
                           double& r4, double& i4, double& r5, double& i5) const
{
    // W6^k for k=0..5
    const double c1 = 0.5, s1 = 0.8660254037844386;   // exp(-2pi/6)
    const double c2 = -0.5, s2 = 0.8660254037844386;  // exp(-4pi/6)
    // W6^3 = -1, W6^4 = conj(W6^2), W6^5 = conj(W6^1)

    // Stage 1: 3-point butterflies (even/odd decomposition)
    double a0r = r0 + r2 + r4, a0i = i0 + i2 + i4;
    double a1r = r0 + c1*r2 - s1*i2 + c2*r4 - s2*i4;
    double a1i = i0 + s1*r2 + c1*i2 + s2*r4 + c2*i4;
    double a2r = r0 + c2*r2 - s2*i2 + c1*r4 - s1*i4;
    double a2i = i0 + s2*r2 + c2*i2 + s1*r4 + c1*i4;

    double b0r = r1 + r3 + r5, b0i = i1 + i3 + i5;
    double b1r = r1 + c1*r3 - s1*i3 + c2*r5 - s2*i5;
    double b1i = i1 + s1*r3 + c1*i3 + s2*r5 + c2*i5;
    double b2r = r1 + c2*r3 - s2*i3 + c1*r5 - s1*i5;
    double b2i = i1 + s2*r3 + c2*i3 + s1*r5 + c1*i5;

    // Stage 2: combine with W6 twiddles
    // W6^0=1, W6^1=(c1,s1), W6^2=(c2,s2), W6^3=(-1,0), W6^4=(c2,-s2), W6^5=(c1,-s1)
    r0 = a0r + b0r;  i0 = a0i + b0i;
    r1 = a1r + c1*b1r - s1*b1i;  i1 = a1i + s1*b1r + c1*b1i;
    r2 = a2r + c2*b2r - s2*b2i;  i2 = a2i + s2*b2r + c2*b2i;
    r3 = a0r - b0r;  i3 = a0i - b0i;
    r4 = a1r + c2*b1r + s2*b1i;  i4 = a1i - s2*b1r + c2*b1i;
    r5 = a2r + c1*b2r + s1*b2i;  i5 = a2i - s1*b2r + c1*b2i;
}

/* ---- Digit-reverse permutation for radix-6 ---- */

void HexFFT6::digitReverse(QVector<double>& re, QVector<double>& im)
{
    int n = m_size;
    QVector<int> revIdx(n, 0);
    for (int i = 0; i < n; ++i) {
        int val = i, rev = 0;
        for (int s = 0; s < m_stages; ++s) {
            rev = rev * 6 + (val % 6);
            val /= 6;
        }
        revIdx[i] = rev;
    }
    for (int i = 0; i < n; ++i) {
        if (revIdx[i] > i) {
            std::swap(re[i], re[revIdx[i]]);
            std::swap(im[i], im[revIdx[i]]);
        }
    }
}

/* ---- Radix-6 Cooley-Tukey ---- */

void HexFFT6::radix6CT(QVector<double>& re, QVector<double>& im, bool inverse)
{
    int n = m_size;
    int groupSize = 1;

    for (int stage = 0; stage < m_stages; ++stage) {
        int numGroups = n / (groupSize * 6);
        int stride = n / 6;

        for (int g = 0; g < numGroups; ++g) {
            int base = g * groupSize * 6;
            for (int k = 0; k < groupSize; ++k) {
                int idx = base + k;

                // Gather 6 points
                double pr[6], pi[6];
                for (int b = 0; b < 6; ++b) {
                    int si = idx + b * groupSize;
                    // Apply twiddle
                    int twIdx = (b * k * stride) % n;
                    double wr = m_twiddles[twIdx * 2];
                    double wi = m_twiddles[twIdx * 2 + 1];
                    if (inverse) wi = -wi;
                    pr[b] = re[si] * wr - im[si] * wi;
                    pi[b] = re[si] * wi + im[si] * wr;
                }

                // Apply 6-point butterfly
                butterfly6(pr[0], pi[0], pr[1], pi[1], pr[2], pi[2],
                           pr[3], pi[3], pr[4], pi[4], pr[5], pi[5]);

                // Scatter
                for (int b = 0; b < 6; ++b) {
                    re[idx + b * groupSize] = pr[b];
                    im[idx + b * groupSize] = pi[b];
                }
            }
        }
        groupSize *= 6;
    }
}

/* ---- Forward transform ---- */

QVector<double> HexFFT6::forward(const QVector<double>& realInput)
{
    QElapsedTimer timer;
    timer.start();

    QVector<double> re(m_size, 0.0), im(m_size, 0.0);
    for (int i = 0; i < qMin(realInput.size(), m_size); ++i)
        re[i] = realInput[i];

    digitReverse(re, im);
    radix6CT(re, im, false);

    // Interleave [re,im,re,im,...]
    QVector<double> result(m_size * 2);
    for (int i = 0; i < m_size; ++i) {
        result[i * 2] = re[i];
        result[i * 2 + 1] = im[i];
    }

    m_stats.totalTransforms++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalTransforms;
    emit transformCompleted(m_size, m_stages, timer.elapsed());
    return result;
}

/* ---- Inverse transform ---- */

QVector<double> HexFFT6::inverse(const QVector<double>& complexInterleaved)
{
    QElapsedTimer timer;
    timer.start();

    QVector<double> re(m_size, 0.0), im(m_size, 0.0);
    for (int i = 0; i < qMin(complexInterleaved.size() / 2, m_size); ++i) {
        re[i] = complexInterleaved[i * 2];
        im[i] = complexInterleaved[i * 2 + 1];
    }

    digitReverse(re, im);
    radix6CT(re, im, true);

    // Scale by 1/N
    for (int i = 0; i < m_size; ++i) re[i] /= m_size;

    m_stats.totalTransforms++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalTransforms;
    return re;
}

/* ---- Magnitude spectrum ---- */

QVector<double> HexFFT6::magnitudeSpectrum(const QVector<double>& realInput)
{
    QVector<double> spec = forward(realInput);
    int half = m_size / 2;
    QVector<double> mag(half);
    for (int i = 0; i < half; ++i) {
        double re = spec[i * 2], im = spec[i * 2 + 1];
        mag[i] = qSqrt(re * re + im * im);
    }
    return mag;
}

/* ---- Power spectrum in dB ---- */

QVector<double> HexFFT6::powerSpectrumDb(const QVector<double>& realInput)
{
    QVector<double> mag = magnitudeSpectrum(realInput);
    QVector<double> db(mag.size());
    for (int i = 0; i < mag.size(); ++i)
        db[i] = 20.0 * qLn(qMax(mag[i], 1e-10)) / qLn(10.0);
    return db;
}

/* ---- Reset ---- */

void HexFFT6::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_twiddles.clear();
}
