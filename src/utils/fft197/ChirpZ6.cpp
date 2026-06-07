/**
 * @file ChirpZ6.cpp
 * @brief ChirpZ6 实现
 *
 * 实现Chirp-Z变换：IIR滤波器实现、连续扫频分析、任意频段高分辨率频谱。
 */

#include "utils/fft197/ChirpZ6.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

ChirpZ6::ChirpZ6(QObject *parent) : QObject(parent) {}
ChirpZ6::~ChirpZ6() = default;

/* ---- Configuration ---- */

void ChirpZ6::setFrequencyRange(double fMin, double fMax, int numPoints)
{
    m_fMin = qBound(0.0, fMin, 0.5);
    m_fMax = qBound(fMin + 0.001, fMax, 0.5);
    m_numPoints = qMax(1, numPoints);
    m_angleStep = (m_fMax - m_fMin) / m_numPoints;
}

void ChirpZ6::setSpiral(double radius, double angleStep)
{
    m_radius = qBound(0.01, radius, 10.0);
    m_angleStep = angleStep;
}

/* ---- Next power of 2 ---- */

int ChirpZ6::nextPow2(int n)
{
    int p = 1;
    while (p < n) p <<= 1;
    return p;
}

/* ---- Internal radix-2 FFT ---- */

void ChirpZ6::fft(QVector<double>& re, QVector<double>& im, bool inverse) const
{
    int N = re.size();
    if (N <= 1) return;

    // Bit-reverse permutation
    for (int i = 1, j = 0; i < N; ++i) {
        int bit = N >> 1;
        while (j & bit) { j ^= bit; bit >>= 1; }
        j ^= bit;
        if (i < j) {
            std::swap(re[i], re[j]);
            std::swap(im[i], im[j]);
        }
    }

    for (int len = 2; len <= N; len <<= 1) {
        double angle = (inverse ? 2.0 : -2.0) * M_PI / len;
        double wRe = qCos(angle), wIm = qSin(angle);
        for (int i = 0; i < N; i += len) {
            double curRe = 1.0, curIm = 0.0;
            for (int j = 0; j < len / 2; ++j) {
                double uRe = re[i + j], uIm = im[i + j];
                double vRe = re[i + j + len/2] * curRe - im[i + j + len/2] * curIm;
                double vIm = re[i + j + len/2] * curIm + im[i + j + len/2] * curRe;
                re[i + j] = uRe + vRe;
                im[i + j] = uIm + vIm;
                re[i + j + len/2] = uRe - vRe;
                im[i + j + len/2] = uIm - vIm;
                double newCurRe = curRe * wRe - curIm * wIm;
                curIm = curRe * wIm + curIm * wRe;
                curRe = newCurRe;
            }
        }
    }

    if (inverse) {
        for (int i = 0; i < N; ++i) {
            re[i] /= N;
            im[i] /= N;
        }
    }
}

/* ---- Compute chirp sequence ---- */

QVector<QVector<double>> ChirpZ6::computeChirp(int N, int M) const
{
    int L = nextPow2(N + M - 1);
    // A = W^(-k) where W = radius * exp(j*angleStep)
    // Chirp sequence: y[n] = x[n] * A^(-n) * W^(n^2/2)
    QVector<QVector<double>> chirp(L, {0.0, 0.0});
    for (int n = 0; n < N + M - 1; ++n) {
        double idx = static_cast<double>(n);
        double angle = M_PI * 2.0 * m_fMin * idx + M_PI * m_angleStep * idx * idx;
        chirp[n] = {qCos(angle), -qSin(angle)};
    }
    return chirp;
}

/* ---- Circular convolution ---- */

QVector<QVector<double>> ChirpZ6::circularConvolve(
    const QVector<QVector<double>>& a,
    const QVector<QVector<double>>& b, int len) const
{
    int L = a.size();
    QVector<double> aRe(L, 0.0), aIm(L, 0.0);
    QVector<double> bRe(L, 0.0), bIm(L, 0.0);

    for (int i = 0; i < L; ++i) {
        aRe[i] = a[i][0]; aIm[i] = a[i][1];
        bRe[i] = b[i][0]; bIm[i] = b[i][1];
    }

    fft(aRe, aIm, false);
    fft(bRe, bIm, false);

    QVector<double> cRe(L, 0.0), cIm(L, 0.0);
    for (int i = 0; i < L; ++i) {
        cRe[i] = aRe[i] * bRe[i] - aIm[i] * bIm[i];
        cIm[i] = aRe[i] * bIm[i] + aIm[i] * bRe[i];
    }

    fft(cRe, cIm, true);

    QVector<QVector<double>> result(len, {0.0, 0.0});
    for (int i = 0; i < len; ++i)
        result[i] = {cRe[i], cIm[i]};
    return result;
}

/* ---- Transform ---- */

QVector<QVector<double>> ChirpZ6::transform(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    int N = input.size();
    int M = m_numPoints;
    if (N == 0 || M == 0) return {};

    int L = nextPow2(N + M - 1);

    // Step 1: Multiply input by A^(-n) * W^(n^2/2)
    QVector<QVector<double>> y(L, {0.0, 0.0});
    for (int n = 0; n < N; ++n) {
        double phase = -2.0 * M_PI * m_fMin * n + M_PI * m_angleStep * n * n;
        double wRe = qCos(phase), wIm = qSin(phase);
        y[n] = {input[n] * wRe, input[n] * wIm};
    }

    // Step 2: Chirp filter h[k] = W^(k^2/2)
    QVector<QVector<double>> h(L, {0.0, 0.0});
    for (int k = 0; k < N + M - 1 && k < L; ++k) {
        double phase = M_PI * m_angleStep * k * k;
        h[k] = {qCos(phase), qSin(phase)};
    }

    // Step 3: Circular convolution y * h
    auto conv = circularConvolve(y, h, M);

    // Step 4: Multiply by W^(k^2/2)
    QVector<QVector<double>> result(M, {0.0, 0.0});
    for (int k = 0; k < M; ++k) {
        double phase = M_PI * m_angleStep * k * k;
        double wRe = qCos(phase), wIm = qSin(phase);
        result[k] = {conv[k][0] * wRe - conv[k][1] * wIm,
                     conv[k][0] * wIm + conv[k][1] * wRe};
    }

    m_stats.totalTransforms++;
    m_stats.inputSize = N;
    m_stats.outputSize = M;
    m_stats.numPoints = M;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalTransforms;

    emit transformCompleted(N, M, timer.elapsed());
    return result;
}

/* ---- Inverse ---- */

QVector<double> ChirpZ6::inverse(const QVector<QVector<double>>& spectrum, int outLen)
{
    int M = spectrum.size();
    if (M == 0) return {};

    // Conjugate and transform back
    QVector<double> conjRe(M), conjIm(M);
    for (int i = 0; i < M; ++i) {
        conjRe[i] = spectrum[i][0];
        conjIm[i] = -spectrum[i][1];
    }

    // Use magnitude as input for simplified inverse
    QVector<double> input(M);
    for (int i = 0; i < M; ++i)
        input[i] = qSqrt(conjRe[i] * conjRe[i] + conjIm[i] * conjIm[i]);

    // Scale and truncate
    QVector<double> result(outLen);
    for (int i = 0; i < outLen; ++i)
        result[i] = (i < M) ? input[i] / M : 0.0;
    return result;
}

/* ---- IIR sweep analysis ---- */

QVector<double> ChirpZ6::iirSweep(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    int N = input.size();
    int M = m_numPoints;

    // IIR filter-based sweep: resonator at each frequency
    QVector<double> magnitude(M, 0.0);
    for (int k = 0; k < M; ++k) {
        double freq = m_fMin + (m_fMax - m_fMin) * k / M;
        double w = 2.0 * M_PI * freq;
        // Second-order IIR resonator: H(z) = 1/(1 - 2*cos(w)*z^-1 + z^-2)
        double a1 = -2.0 * qCos(w);
        double a2 = 1.0;
        double w1 = 0.0, w2 = 0.0;
        double re = 0.0, im = 0.0;
        for (int n = 0; n < N; ++n) {
            double w0 = input[n] - a1 * w1 - a2 * w2;
            re += w0 * qCos(w * n);
            im += w0 * qSin(w * n);
            w2 = w1; w1 = w0;
        }
        magnitude[k] = qSqrt(re * re + im * im) / N;
    }

    m_stats.totalTransforms++;
    m_stats.inputSize = N;
    m_stats.outputSize = M;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalTransforms;

    emit transformCompleted(N, M, timer.elapsed());
    return magnitude;
}

/* ---- Frequency bins ---- */

QVector<double> ChirpZ6::frequencyBins() const
{
    QVector<double> bins(m_numPoints);
    for (int i = 0; i < m_numPoints; ++i)
        bins[i] = m_fMin + (m_fMax - m_fMin) * i / m_numPoints;
    return bins;
}

/* ---- Reset ---- */

void ChirpZ6::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
