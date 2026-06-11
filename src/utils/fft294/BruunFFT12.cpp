/**
 * @file BruunFFT12.cpp
 * @brief BruunFFT12 实现
 *
 * 实现Bruun FFT：多项式余因子分解与递归余弦调制实现2的幂次实值变换。
 */

#include "utils/fft294/BruunFFT12.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

BruunFFT12::BruunFFT12(QObject *parent)
    : QObject(parent)
{
    precompute();
}

BruunFFT12::~BruunFFT12() = default;

/* ---- Configuration ---- */

void BruunFFT12::setSize(int N)
{
    // Round to nearest power of 2
    int p = 1;
    while (p < N) p *= 2;
    m_N = qBound(4, p, 1 << 20);
    precompute();
}

/* ---- Precompute Bruun twiddle factors ---- */

void BruunFFT12::precompute()
{
    m_twiddles.clear();
    int stages = 0;
    { int n = m_N; while (n > 2) { n /= 2; stages++; } }

    m_twiddles.resize(stages);
    for (int s = 0; s < stages; ++s) {
        int halfN = m_N >> (s + 1);
        m_twiddles[s].resize(halfN);
        for (int k = 0; k < halfN; ++k) {
            double angle = -2.0 * M_PI * k / (m_N >> s);
            m_twiddles[s][k] = {qCos(angle), qSin(angle)};
        }
    }
}

/* ---- 2-point butterfly ---- */

void BruunFFT12::butterfly2(double* r0, double* i0,
                              double* r1, double* i1) const
{
    double tr = *r0 + *r1;
    double ti = *i0 + *i1;
    *r1 = *r0 - *r1;
    *i1 = *i0 - *i1;
    *r0 = tr;
    *i0 = ti;
}

/* ---- Recursive Bruun butterfly on real data ---- */

void BruunFFT12::bruunReal(double* real, double* imag, int N, int stage) const
{
    if (N == 2) {
        butterfly2(&real[0], &imag[0], &real[1], &imag[1]);
        return;
    }

    int halfN = N / 2;

    // Bruun's polynomial residue factorization:
    // Split into even/odd via z^{N/2} ± 1 residue pairs
    // First do the even/odd decomposition
    QVector<double> evenR(halfN), evenI(halfN);
    QVector<double> oddR(halfN), oddI(halfN);

    for (int i = 0; i < halfN; ++i) {
        evenR[i] = real[2 * i];
        evenI[i] = imag[2 * i];
        oddR[i] = real[2 * i + 1];
        oddI[i] = imag[2 * i + 1];
    }

    // Recursive decomposition of even and odd parts
    bruunReal(evenR.data(), evenI.data(), halfN, stage + 1);
    bruunReal(oddR.data(), oddI.data(), halfN, stage + 1);

    // Combine with cosine modulation twiddles
    // Bruun uses z^{N/2}-1 and z^{N/2}+1 factorization
    if (stage < m_twiddles.size()) {
        const auto& tw = m_twiddles[stage];
        int twSize = tw.size();
        for (int k = 0; k < halfN; ++k) {
            double wr = tw[k % twSize].cosVal;
            double wi = tw[k % twSize].sinVal;

            // Twiddle odd output
            double tR = oddR[k] * wr - oddI[k] * wi;
            double tI = oddR[k] * wi + oddI[k] * wr;

            real[k] = evenR[k] + tR;
            imag[k] = evenI[k] + tI;
            real[k + halfN] = evenR[k] - tR;
            imag[k + halfN] = evenI[k] - tI;
        }
    } else {
        // Fallback: simple combine without twiddle
        for (int k = 0; k < halfN; ++k) {
            real[k] = evenR[k] + oddR[k];
            imag[k] = evenI[k] + oddI[k];
            real[k + halfN] = evenR[k] - oddR[k];
            imag[k + halfN] = evenI[k] - oddI[k];
        }
    }
}

/* ---- Inverse Bruun ---- */

void BruunFFT12::bruunRealInverse(double* real, double* imag, int N, int stage) const
{
    if (N == 2) {
        butterfly2(&real[0], &imag[0], &real[1], &imag[1]);
        return;
    }

    int halfN = N / 2;

    // Undo twiddle and combine
    QVector<double> evenR(halfN), evenI(halfN);
    QVector<double> oddR(halfN), oddI(halfN);

    if (stage < m_twiddles.size()) {
        const auto& tw = m_twiddles[stage];
        int twSize = tw.size();
        for (int k = 0; k < halfN; ++k) {
            evenR[k] = (real[k] + real[k + halfN]) * 0.5;
            evenI[k] = (imag[k] + imag[k + halfN]) * 0.5;

            double dR = (real[k] - real[k + halfN]) * 0.5;
            double dI = (imag[k] - imag[k + halfN]) * 0.5;

            // Conjugate twiddle
            double wr = tw[k % twSize].cosVal;
            double wi = -tw[k % twSize].sinVal;
            oddR[k] = dR * wr - dI * wi;
            oddI[k] = dR * wi + dI * wr;
        }
    } else {
        for (int k = 0; k < halfN; ++k) {
            evenR[k] = (real[k] + real[k + halfN]) * 0.5;
            evenI[k] = (imag[k] + imag[k + halfN]) * 0.5;
            oddR[k] = (real[k] - real[k + halfN]) * 0.5;
            oddI[k] = (imag[k] - imag[k + halfN]) * 0.5;
        }
    }

    // Recursive inverse
    bruunRealInverse(evenR.data(), evenI.data(), halfN, stage + 1);
    bruunRealInverse(oddR.data(), oddI.data(), halfN, stage + 1);

    // Interleave back
    for (int i = 0; i < halfN; ++i) {
        real[2 * i] = evenR[i];
        imag[2 * i] = evenI[i];
        real[2 * i + 1] = oddR[i];
        imag[2 * i + 1] = oddI[i];
    }
}

/* ---- Forward transform ---- */

BruunFFT12::FFTResult BruunFFT12::forward(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    FFTResult result;
    result.real.resize(m_N, 0.0);
    result.imag.resize(m_N, 0.0);

    // Copy input to working buffers
    QVector<double> wr(m_N, 0.0), wi(m_N, 0.0);
    for (int i = 0; i < qMin(input.size(), m_N); ++i)
        wr[i] = input[i];

    // Apply Bruun real-valued FFT
    bruunReal(wr.data(), wi.data(), m_N, 0);

    result.real = wr;
    result.imag = wi;

    // Compute peak magnitude
    double peak = 0.0;
    for (int i = 0; i < m_N; ++i) {
        double mag = qSqrt(wr[i] * wr[i] + wi[i] * wi[i]);
        peak = qMax(peak, mag);
    }
    result.peakMagnitude = peak;

    double elapsed = timer.elapsed();
    m_stats.transformSize = m_N;
    m_stats.totalTransforms++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalTransforms;

    emit transformDone(m_N, peak, elapsed);
    return result;
}

/* ---- Inverse transform ---- */

BruunFFT12::FFTResult BruunFFT12::inverse(const QVector<double>& realIn,
                                             const QVector<double>& imagIn)
{
    QElapsedTimer timer;
    timer.start();

    FFTResult result;
    QVector<double> wr(m_N, 0.0), wi(m_N, 0.0);
    for (int i = 0; i < qMin(realIn.size(), m_N); ++i) {
        wr[i] = realIn[i];
        wi[i] = -imagIn[i]; // Conjugate for inverse
    }

    bruunRealInverse(wr.data(), wi.data(), m_N, 0);

    // Scale by 1/N and conjugate
    for (int i = 0; i < m_N; ++i) {
        result.real.append(wr[i] / m_N);
        result.imag.append(-wi[i] / m_N);
    }

    double peak = 0.0;
    for (int i = 0; i < m_N; ++i) {
        double mag = qSqrt(result.real[i] * result.real[i] + result.imag[i] * result.imag[i]);
        peak = qMax(peak, mag);
    }
    result.peakMagnitude = peak;

    double elapsed = timer.elapsed();
    m_stats.totalTransforms++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalTransforms;

    emit transformDone(m_N, peak, elapsed);
    return result;
}

/* ---- Reset ---- */

void BruunFFT12::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
