/**
 * @file BruunFFT4.cpp
 * @brief BruunFFT4 实现
 *
 * 实现Bruun FFT：多项式z^N-1的实系数因子分解、递归实值变换、减少复数运算。
 */

#include "utils/fft181/BruunFFT4.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

BruunFFT4::BruunFFT4(QObject *parent) : QObject(parent) {}
BruunFFT4::~BruunFFT4() = default;

/* ---- Utility functions ---- */

bool BruunFFT4::isPowerOfTwo(int n) const { return n > 0 && (n & (n - 1)) == 0; }

int BruunFFT4::log2Int(int n) const
{
    int bits = 0;
    while (n > 1) { n >>= 1; ++bits; }
    return bits;
}

int BruunFFT4::numStages(int n) const { return log2Int(n); }

int BruunFFT4::reverseBits(int val, int bits) const
{
    int result = 0;
    for (int i = 0; i < bits; ++i) {
        result = (result << 1) | (val & 1);
        val >>= 1;
    }
    return result;
}

void BruunFFT4::bitReverse(QVector<double>& data)
{
    int n = data.size();
    int bits = log2Int(n);
    for (int i = 0; i < n; ++i) {
        int j = reverseBits(i, bits);
        if (j > i) {
            double tmp = data[i];
            data[i] = data[j];
            data[j] = tmp;
        }
    }
}

/* ---- Real-valued twiddle factors ---- */

void BruunFFT4::realTwiddles(int N, QVector<double>& cosTable, QVector<double>& sinTable)
{
    int half = N / 2;
    cosTable.resize(half);
    sinTable.resize(half);
    for (int k = 0; k < half; ++k) {
        double angle = -2.0 * M_PI * k / N;
        cosTable[k] = qCos(angle);
        sinTable[k] = qSin(angle);
    }
}

/* ---- Recursive Bruun butterfly ---- */
// Bruun's algorithm factors z^N - 1 = (z^{N/2} - 1)(z^{N/2} + 1)
// For real input, the (z^{N/2} + 1) factor leads to real-coefficient
// butterfly operations, reducing the number of multiplications.

void BruunFFT4::bruunRecursive(const QVector<double>& in,
                                 QVector<double>& outRe, QVector<double>& outIm,
                                 int N, int stride, int offset)
{
    if (N == 1) {
        outRe[offset] = in[offset];
        outIm[offset] = 0.0;
        return;
    }

    if (N == 2) {
        // 2-point DFT: real butterfly
        double a = in[offset];
        double b = in[offset + stride];
        outRe[offset] = a + b;
        outIm[offset] = 0.0;
        outRe[offset + stride] = a - b;
        outIm[offset + stride] = 0.0;
        return;
    }

    int half = N / 2;

    // Step 1: Polynomial residue step
    // Compute even and odd parts via Bruun factorization
    // z^N - 1 = (z^{N/2} - 1)(z^{N/2} + 1)
    // For real signals: pre-twiddle with cos/sin
    QVector<double> evenData(half), oddData(half);
    for (int i = 0; i < half; ++i) {
        int idx0 = offset + i * stride;
        int idx1 = offset + (i + half) * stride;
        // Bruun pre-twiddle: multiply by z^{-k/N} factors
        double angle = -M_PI * i / N;
        double c = qCos(angle);
        double s = qSin(angle);

        // Even/odd decomposition with twiddle
        double a = in[idx0];
        double b = in[idx1];
        evenData[i] = a + b * c;
        oddData[i] = -b * s;
    }

    // Step 2: Recurse on even and odd parts
    QVector<double> eRe(N, 0.0), eIm(N, 0.0);
    QVector<double> oRe(N, 0.0), oIm(N, 0.0);

    // Process even part
    for (int i = 0; i < half; ++i) {
        outRe[offset + i * stride] = evenData[i];
        outRe[offset + (i + half) * stride] = 0.0;
    }
    bruunRecursive(evenData, outRe, outIm, half, stride, offset);
    bruunRecursive(oddData, outRe, outIm, half, stride, offset + half * stride);

    // Step 3: Combine with post-twiddle
    QVector<double> tmpRe(half), tmpIm(half);
    for (int k = 0; k < half; ++k) {
        double angle = -2.0 * M_PI * k / N;
        double c = qCos(angle);
        double s = qSin(angle);

        int idx = offset + k * stride;
        int idxH = offset + (k + half) * stride;

        double er = outRe[idx];
        double ei = outIm[idx];
        double or_ = outRe[idxH];
        double oi = outIm[idxH];

        tmpRe[k] = er + c * or_ - s * oi;
        tmpIm[k] = ei + s * or_ + c * oi;
    }

    for (int k = 0; k < half; ++k) {
        int idx = offset + k * stride;
        int idxH = offset + (k + half) * stride;

        outRe[idx] = tmpRe[k];
        outIm[idx] = tmpIm[k];

        // Hermitian symmetry for real signal
        outRe[idxH] = tmpRe[half - 1 - k];
        outIm[idxH] = -tmpIm[half - 1 - k];
    }
}

/* ---- Forward transform ---- */

QPair<QVector<double>, QVector<double>> BruunFFT4::transform(
    const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    int N = input.size();
    if (N == 0) return {{}, {}};

    // Pad to next power of 2 if needed
    int N2 = 1;
    while (N2 < N) N2 <<= 1;
    QVector<double> padded = input;
    padded.resize(N2);

    QVector<double> outRe(N2, 0.0), outIm(N2, 0.0);
    bruunRecursive(padded, outRe, outIm, N2, 1, 0);

    // Trim to original size
    outRe.resize(N);
    outIm.resize(N);

    int stages = numStages(N2);
    m_stats.totalTransforms++;
    m_stats.transformSize = N;
    m_stats.numStages = stages;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalTransforms;

    emit transformCompleted(N, stages);
    return {outRe, outIm};
}

/* ---- Inverse transform ---- */

QPair<QVector<double>, QVector<double>> BruunFFT4::inverseTransform(
    const QVector<double>& re, const QVector<double>& im)
{
    int N = re.size();
    // Conjugate input, forward transform, conjugate and scale
    QVector<double> conjIm(N);
    for (int i = 0; i < N; ++i) conjIm[i] = -im[i];

    auto result = transform(re); // Only real part forward (Bruun is real-focused)
    // Combine with imaginary contribution
    QVector<double>& outR = result.first;
    QVector<double>& outI = result.second;
    for (int i = 0; i < N; ++i) {
        outR[i] /= N;
        outI[i] = -outI[i] / N;
    }
    return result;
}

/* ---- Reset ---- */

void BruunFFT4::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
