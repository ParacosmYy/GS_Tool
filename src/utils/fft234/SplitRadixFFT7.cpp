/**
 * @file SplitRadixFFT7.cpp
 * @brief SplitRadixFFT7 实现
 *
 * 实现分裂基FFT：迭代就地计算与预计算旋转因子缓存最优访问。
 */

#include "utils/fft234/SplitRadixFFT7.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

SplitRadixFFT7::SplitRadixFFT7(QObject *parent) : QObject(parent) {}
SplitRadixFFT7::~SplitRadixFFT7() = default;

/* ---- Reverse bits ---- */

int SplitRadixFFT7::revBits(int x, int bits) const
{
    int r = 0;
    for (int i = 0; i < bits; ++i) {
        r = (r << 1) | (x & 1);
        x >>= 1;
    }
    return r;
}

/* ---- Bit-reversal permutation ---- */

void SplitRadixFFT7::bitReverse(double* re, double* im) const
{
    for (int i = 0; i < m_n; ++i) {
        int j = revBits(i, m_log2n);
        if (i < j) {
            std::swap(re[i], re[j]);
            std::swap(im[i], im[j]);
        }
    }
}

/* ---- Precompute twiddle factors ---- */

void SplitRadixFFT7::precomputeTwiddles()
{
    // Split-radix uses twiddle factors at N/4, N/8, etc.
    int qn = m_n / 4;
    m_twiddle.resize(2 * qn);  // interleaved cos, sin
    double sign = -1.0;  // forward transform sign
    for (int k = 0; k < qn; ++k) {
        double angle = sign * 2.0 * M_PI * k / m_n;
        m_twiddle[2 * k] = qCos(angle);
        m_twiddle[2 * k + 1] = qSin(angle);
    }
}

/* ---- Configure ---- */

bool SplitRadixFFT7::configure(int n)
{
    // Verify power of 2
    if (n < 4) return false;
    int val = n;
    while (val > 1) {
        if (val & 1) return false;
        val >>= 1;
    }

    m_n = n;
    m_log2n = 0;
    val = n;
    while (val > 1) { m_log2n++; val >>= 1; }

    m_stats.transformSize = n;
    precomputeTwiddles();
    return true;
}

/* ---- Iterative in-place split-radix FFT ---- */

void SplitRadixFFT7::splitRadixCore(double* re, double* im, int n, bool inverse) const
{
    int log2n = 0;
    { int v = n; while (v > 1) { log2n++; v >>= 1; } }

    double signVal = inverse ? 1.0 : -1.0;

    // L-shaped butterfly: split-radix combines radix-2 and radix-4
    // Stage 0: radix-2 on first half
    {
        int half = n / 2;
        double angle0 = signVal * M_PI;
        for (int i = 0; i < half; ++i) {
            double tR = re[i + half] * qCos(angle0 * i / half)
                        - im[i + half] * qSin(angle0 * i / half);
            double tI = re[i + half] * qSin(angle0 * i / half)
                        + im[i + half] * qCos(angle0 * i / half);
            re[i + half] = re[i] - tR;
            im[i + half] = im[i] - tI;
            re[i] += tR;
            im[i] += tI;
        }
    }

    // Subsequent stages: radix-4 butterfly (L-shaped)
    int quarter = n / 4;
    int stride = quarter;

    while (stride >= 1) {
        int dual = stride * 2;
        for (int j = 0; j < stride; ++j) {
            // Compute twiddle for this position
            int twIdx = j % (m_n / 4);
            double twR = m_twiddle[2 * twIdx];
            double twI = m_twiddle[2 * twIdx + 1] * (inverse ? -1.0 : 1.0);

            for (int i = j; i < n; i += 4 * stride) {
                int i1 = i + stride;
                int i2 = i + dual;
                int i3 = i2 + stride;

                // Apply twiddle to i1, i3
                double x1R = re[i1] * twR - im[i1] * twI;
                double x1I = re[i1] * twI + im[i1] * twR;
                double x3R = re[i3] * twR - im[i3] * twI;
                double x3I = re[i3] * twI + im[i3] * twR;

                // Compute twiddle W^{3j}
                double twR3 = twR * twR - twI * twI;
                double twI3 = 2.0 * twR * twI;
                double y3R = re[i3] * twR3 - im[i3] * twI3;
                double y3I = re[i3] * twI3 + im[i3] * twR3;

                // L-shaped butterfly
                double sum1R = re[i] - x1R;
                double sum1I = im[i] - x1I;
                double diff1R = re[i] + x1R;
                double diff1I = im[i] + x1I;

                double sum2R = re[i2] - y3I;
                double sum2I = im[i2] + y3R;
                double diff2R = re[i2] + y3I;
                double diff2I = im[i2] - y3R;

                re[i] = sum1R + diff2R;
                im[i] = sum1I + diff2I;
                re[i1] = diff1R - sum2I;
                im[i1] = diff1I + sum2R;
                re[i2] = sum1R - diff2R;
                im[i2] = sum1I - diff2I;
                re[i3] = diff1R + sum2I;
                im[i3] = diff1I - sum2R;
            }
        }
        stride >>= 1;
    }
}

/* ---- Forward transform ---- */

QVector<double> SplitRadixFFT7::forward(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    int n = m_n;
    QVector<double> re(n, 0.0), im(n, 0.0);

    // De-interleave input
    for (int i = 0; i < qMin(input.size() / 2, n); ++i) {
        re[i] = input[2 * i];
        im[i] = input[2 * i + 1];
    }

    // Bit-reverse permutation
    bitReverse(re.data(), im.data());

    // Split-radix core
    splitRadixCore(re.data(), im.data(), n, false);

    // Count butterflies
    m_stats.numButterflies = 0;
    int s = n;
    while (s >= 4) {
        m_stats.numButterflies += n / 4;
        s /= 4;
    }

    // Re-interleave output
    QVector<double> output(2 * n);
    for (int i = 0; i < n; ++i) {
        output[2 * i] = re[i];
        output[2 * i + 1] = im[i];
    }

    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit transformCompleted(n, timer.elapsed());
    return output;
}

/* ---- Inverse transform ---- */

QVector<double> SplitRadixFFT7::inverse(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    int n = m_n;
    QVector<double> re(n, 0.0), im(n, 0.0);

    for (int i = 0; i < qMin(input.size() / 2, n); ++i) {
        re[i] = input[2 * i];
        im[i] = input[2 * i + 1];
    }

    bitReverse(re.data(), im.data());
    splitRadixCore(re.data(), im.data(), n, true);

    // Scale by 1/N
    QVector<double> output(2 * n);
    for (int i = 0; i < n; ++i) {
        output[2 * i] = re[i] / n;
        output[2 * i + 1] = im[i] / n;
    }

    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit transformCompleted(n, timer.elapsed());
    return output;
}

/* ---- Reset ---- */

void SplitRadixFFT7::resetStatistics()
{
    m_twiddle.clear();
    m_n = 0;
    m_log2n = 0;
    m_stats = Stats{};
    m_timeSum = 0.0;
}
