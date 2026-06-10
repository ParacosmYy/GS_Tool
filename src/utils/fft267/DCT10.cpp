/**
 * @file DCT10.cpp
 * @brief DCT10 实现
 *
 * 实现离散余弦变换：II型FFT前后预处理与偶扩展正交DCT快速计算。
 */

#include "utils/fft267/DCT10.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

DCT10::DCT10(QObject *parent)
    : QObject(parent) {}

DCT10::~DCT10() = default;

/* ---- Utility ---- */

int DCT10::nextPow2(int n)
{
    int p = 1;
    while (p < n) p <<= 1;
    return qMax(2, p);
}

int DCT10::numStages(int n)
{
    int s = 0;
    while (n > 1) { n >>= 1; s++; }
    return s;
}

/* ---- Bit-reversal permutation ---- */

void DCT10::bitReverse(QVector<double>& data) const
{
    int n = data.size();
    int bits = 0;
    for (int tmp = n; tmp > 1; tmp >>= 1) bits++;

    for (int i = 0; i < n; ++i) {
        int rev = 0;
        for (int b = 0; b < bits; ++b) {
            if (i & (1 << b)) rev |= (1 << (bits - 1 - b));
        }
        if (rev > i) std::swap(data[i], data[rev]);
    }
}

/* ---- Radix-2 FFT ---- */

void DCT10::fftRadix2(QVector<double>& re, QVector<double>& im) const
{
    int n = re.size();
    int stages = numStages(n);

    // Bit-reversal
    int bits = stages;
    for (int i = 0; i < n; ++i) {
        int rev = 0;
        for (int b = 0; b < bits; ++b)
            if (i & (1 << b)) rev |= (1 << (bits - 1 - b));
        if (rev > i) {
            std::swap(re[i], re[rev]);
            std::swap(im[i], im[rev]);
        }
    }

    // Butterfly stages
    for (int s = 1; s <= stages; ++s) {
        int m = 1 << s;
        int halfM = m >> 1;
        double angle = -2.0 * M_PI / m;

        for (int k = 0; k < n; k += m) {
            for (int j = 0; j < halfM; ++j) {
                double wAngle = angle * j;
                double wRe = qCos(wAngle);
                double wIm = qSin(wAngle);

                double tRe = wRe * re[k + j + halfM] - wIm * im[k + j + halfM];
                double tIm = wRe * im[k + j + halfM] + wIm * re[k + j + halfM];

                re[k + j + halfM] = re[k + j] - tRe;
                im[k + j + halfM] = im[k + j] - tIm;
                re[k + j] += tRe;
                im[k + j] += tIm;
            }
        }
    }
}

/* ---- Forward DCT-II (orthonormal) ---- */

QVector<double> DCT10::transform(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    int n = nextPow2(input.size());
    int stages = numStages(n);

    // DCT-II via even-extension into 2N-point, then take first N results
    // Direct O(N^2) for simplicity; fast path uses FFT below
    QVector<double> result(n, 0.0);
    double scale = qSqrt(2.0 / n);

    for (int k = 0; k < n; ++k) {
        double sum = 0.0;
        for (int i = 0; i < n; ++i) {
            double x = (i < input.size()) ? input[i] : 0.0;
            sum += x * qCos(M_PI * (2.0 * i + 1) * k / (2.0 * n));
        }
        result[k] = sum * scale;
    }
    // First coefficient uses different scale for orthonormality
    result[0] *= qSqrt(0.5);

    double elapsed = timer.elapsed();
    m_stats.transformSize = n;
    m_stats.numStages = stages;
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit transformComputed(n, stages, elapsed);
    return result;
}

/* ---- DCT-II via FFT pre/post-processing ---- */

QVector<double> DCT10::transformViaFFT(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    int N = nextPow2(input.size());
    int stages = numStages(N);

    // Reorder input: even indices first, then odd indices reversed
    QVector<double> re(N, 0.0);
    QVector<double> im(N, 0.0);

    for (int i = 0; i < N; ++i) {
        if (2 * i < N && 2 * i < input.size())
            re[i] = input[2 * i];
        else if (2 * i < N)
            re[i] = 0.0;
    }
    for (int i = 0; i < N / 2; ++i) {
        int srcIdx = N - 1 - 2 * i;
        if (srcIdx < input.size())
            re[N / 2 + i] = input[srcIdx];
    }

    // Apply N-point FFT
    fftRadix2(re, im);

    // Post-processing: complex multiplication with twiddle factors
    QVector<double> result(N);
    double scale = qSqrt(2.0 / N) * 0.5;

    for (int k = 0; k < N; ++k) {
        double phase = M_PI * k / (2.0 * N);
        double twRe = qCos(phase);
        double twIm = -qSin(phase);
        result[k] = (re[k] * twRe - im[k] * twIm) * 2.0 * scale;
    }
    result[0] *= qSqrt(2.0);

    double elapsed = timer.elapsed();
    m_stats.transformSize = N;
    m_stats.numStages = stages;
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit transformComputed(N, stages, elapsed);
    return result;
}

/* ---- Inverse DCT-II (orthonormal, = DCT-III) ---- */

QVector<double> DCT10::inverseTransform(const QVector<double>& spectrum)
{
    QElapsedTimer timer;
    timer.start();

    int n = spectrum.size();

    QVector<double> result(n, 0.0);
    double scale = qSqrt(2.0 / n);

    for (int i = 0; i < n; ++i) {
        double sum = spectrum[0] * qSqrt(0.5);
        for (int k = 1; k < n; ++k)
            sum += spectrum[k] * qCos(M_PI * k * (2.0 * i + 1) / (2.0 * n));
        result[i] = sum * scale;
    }

    double elapsed = timer.elapsed();
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    return result;
}

/* ---- Reset ---- */

void DCT10::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
