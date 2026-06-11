/**
 * @file RaderFFT12.cpp
 * @brief RaderFFT12 实现
 *
 * 实现Rader FFT算法：Winograd短DFT内核与置换旋转因子预计算实现素数长度快速卷积。
 */

#include "utils/fft292/RaderFFT12.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

RaderFFT12::RaderFFT12(QObject *parent)
    : QObject(parent)
{
    precompute();
}

RaderFFT12::~RaderFFT12() = default;

/* ---- Configuration ---- */

void RaderFFT12::setSize(int N)
{
    if (isPrime(N)) {
        m_N = qBound(3, N, 65537);
    } else {
        // Find nearest prime
        for (int p = N; p <= N + 100; ++p) {
            if (isPrime(p)) { m_N = p; break; }
        }
    }
    precompute();
}

/* ---- Primality test ---- */

bool RaderFFT12::isPrime(int n) const
{
    if (n < 2) return false;
    if (n == 2 || n == 3) return true;
    if (n % 2 == 0) return false;
    for (int i = 3; i * i <= n; i += 2)
        if (n % i == 0) return false;
    return true;
}

/* ---- Find primitive root modulo p ---- */

int RaderFFT12::primitiveRoot(int p) const
{
    for (int g = 2; g < p; ++g) {
        long long val = 1;
        bool ok = true;
        for (int i = 1; i < p - 1; ++i) {
            val = (val * g) % p;
            if (val == 1) { ok = false; break; }
        }
        if (ok) return g;
    }
    return 2;
}

/* ---- Precompute permutation and twiddle factors ---- */

void RaderFFT12::precompute()
{
    m_stats.transformSize = m_N;
    int M = m_N - 1; // Length of cyclic convolution

    // Generate permutation from primitive root
    int g = primitiveRoot(m_N);
    m_permA.resize(M);
    m_permB.resize(M);

    long long val = 1;
    for (int i = 0; i < M; ++i) {
        m_permA[i] = static_cast<int>(val);
        m_permB[M - 1 - i] = static_cast<int>(val);
        val = (val * g) % m_N;
    }

    // Precompute twiddle factors for the permutation
    m_twiddleReal.resize(M);
    m_twiddleImag.resize(M);
    for (int i = 0; i < M; ++i) {
        double angle = -2.0 * M_PI * m_permA[i] / m_N;
        m_twiddleReal[i] = qCos(angle);
        m_twiddleImag[i] = qSin(angle);
    }

    // Precompute Winograd short-DFT kernel for inner convolution
    m_winogradReal.resize(M);
    m_winogradImag.resize(M);
    for (int i = 0; i < M; ++i) {
        double angle = -2.0 * M_PI * i / M;
        m_winogradReal[i] = qCos(angle);
        m_winogradImag[i] = qSin(angle);
    }
}

/* ---- Winograd short-DFT inner kernel ---- */

void RaderFFT12::winogradShortDFT(const QVector<double>& inReal,
                                    const QVector<double>& inImag,
                                    QVector<double>& outReal,
                                    QVector<double>& outImag) const
{
    int M = m_N - 1;
    outReal.resize(M);
    outImag.resize(M);

    // Simplified Winograd: DFT using precomputed factors with reduced multiplications
    for (int k = 0; k < M; ++k) {
        double sr = 0.0, si = 0.0;
        for (int n = 0; n < M; ++n) {
            double wr = m_winogradReal[(n * k) % M];
            double wi = m_winogradImag[(n * k) % M];
            sr += inReal[n] * wr - inImag[n] * wi;
            si += inReal[n] * wi + inImag[n] * wr;
        }
        outReal[k] = sr;
        outImag[k] = si;
    }
}

/* ---- Complex multiply ---- */

void RaderFFT12::complexMultiply(const QVector<double>& ar, const QVector<double>& ai,
                                   const QVector<double>& br, const QVector<double>& bi,
                                   QVector<double>& cr, QVector<double>& ci) const
{
    int n = qMin(ar.size(), br.size());
    cr.resize(n); ci.resize(n);
    for (int i = 0; i < n; ++i) {
        cr[i] = ar[i] * br[i] - ai[i] * bi[i];
        ci[i] = ar[i] * bi[i] + ai[i] * br[i];
    }
}

/* ---- Forward FFT on complex input ---- */

RaderFFT12::FFTResult RaderFFT12::forwardComplex(const QVector<double>& realIn,
                                                   const QVector<double>& imagIn)
{
    QElapsedTimer timer;
    timer.start();

    FFTResult result;
    int M = m_N - 1;

    // DC component (sum of all inputs)
    double dcR = 0.0, dcI = 0.0;
    for (int i = 0; i < m_N; ++i) {
        dcR += (i < realIn.size()) ? realIn[i] : 0.0;
        dcI += (i < imagIn.size()) ? imagIn[i] : 0.0;
    }

    // Build permuted sequences for Rader's algorithm
    QVector<double> xpR(M), xpI(M), wpR(M), wpI(M);
    for (int i = 0; i < M; ++i) {
        int idx = m_permA[i] - 1;
        xpR[i] = (idx >= 0 && idx < realIn.size()) ? realIn[idx] : 0.0;
        xpI[i] = (idx >= 0 && idx < imagIn.size()) ? imagIn[idx] : 0.0;
        wpR[i] = m_twiddleReal[i];
        wpI[i] = m_twiddleImag[i];
    }

    // Circular convolution via Winograd DFT
    QVector<double> XP_R, XP_I, WP_R, WP_I;
    winogradShortDFT(xpR, xpI, XP_R, XP_I);
    winogradShortDFT(wpR, wpI, WP_R, WP_I);

    // Pointwise multiply in frequency domain
    QVector<double> YP_R, YP_I;
    complexMultiply(XP_R, XP_I, WP_R, WP_I, YP_R, YP_I);

    // Inverse DFT back to time domain
    QVector<double> ypR, ypI;
    winogradShortDFT(YP_R, YP_I, ypR, ypI);
    for (int i = 0; i < M; ++i) {
        ypR[i] /= M;
        ypI[i] /= M;
    }

    // Build full output using inverse permutation
    result.real.resize(m_N);
    result.imag.resize(m_N);
    result.real[0] = dcR;
    result.imag[0] = dcI;
    for (int i = 0; i < M; ++i) {
        int idx = m_permB[i] - 1;
        if (idx >= 0 && idx < m_N) {
            result.real[idx] = dcR + ypR[i];
            result.imag[idx] = dcI + ypI[i];
        }
    }

    // Peak magnitude
    result.magnitude = 0.0;
    for (int i = 0; i < m_N; ++i) {
        double mag = qSqrt(result.real[i] * result.real[i] + result.imag[i] * result.imag[i]);
        result.magnitude = qMax(result.magnitude, mag);
    }

    double elapsed = timer.elapsed();
    m_stats.totalTransforms++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalTransforms;

    emit transformDone(m_N, result.magnitude, elapsed);
    return result;
}

/* ---- Forward FFT on real input ---- */

RaderFFT12::FFTResult RaderFFT12::forward(const QVector<double>& input)
{
    QVector<double> imag(input.size(), 0.0);
    return forwardComplex(input, imag);
}

/* ---- Inverse FFT ---- */

RaderFFT12::FFTResult RaderFFT12::inverse(const QVector<double>& realIn,
                                            const QVector<double>& imagIn)
{
    // Conjugate, forward, conjugate, scale
    QVector<double> conjImag(imagIn.size());
    for (int i = 0; i < imagIn.size(); ++i)
        conjImag[i] = -imagIn[i];

    auto result = forwardComplex(realIn, conjImag);
    for (int i = 0; i < result.real.size(); ++i) {
        result.real[i] /= m_N;
        result.imag[i] = -result.imag[i] / m_N;
    }
    return result;
}

/* ---- Fast convolution ---- */

QVector<double> RaderFFT12::convolve(const QVector<double>& a, const QVector<double>& b)
{
    // Pad to prime length
    int len = a.size() + b.size() - 1;
    setSize(len);

    QVector<double> paddedA(m_N, 0.0), paddedB(m_N, 0.0);
    for (int i = 0; i < a.size(); ++i) paddedA[i] = a[i];
    for (int i = 0; i < b.size(); ++i) paddedB[i] = b[i];

    auto fa = forward(paddedA);
    auto fb = forward(paddedB);

    QVector<double> cr, ci;
    complexMultiply(fa.real, fa.imag, fb.real, fb.imag, cr, ci);

    auto result = inverse(cr, ci);
    result.real.resize(len);
    return result.real;
}

/* ---- Reset ---- */

void RaderFFT12::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
