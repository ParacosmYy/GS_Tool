/**
 * @file RaderFFT8.cpp
 * @brief RaderFFT8 实现
 *
 * 实现Rader FFT：Bluestein chirp-z预计算与素数长度DFT循环卷积映射。
 */

#include "utils/fft236/RaderFFT8.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

RaderFFT8::RaderFFT8(QObject *parent) : QObject(parent) {}
RaderFFT8::~RaderFFT8() = default;

/* ---- Static helpers ---- */

bool RaderFFT8::isPrime(int n)
{
    if (n < 2) return false;
    if (n < 4) return true;
    if (n % 2 == 0 || n % 3 == 0) return false;
    for (int i = 5; i * i <= n; i += 6) {
        if (n % i == 0 || n % (i + 2) == 0) return false;
    }
    return true;
}

int RaderFFT8::nextPow2(int n)
{
    int p = 1;
    while (p < n) p <<= 1;
    return p;
}

/* ---- Complex arithmetic ---- */

RaderFFT8::Complex RaderFFT8::cmul(const Complex& a, const Complex& b)
{
    return {a.re * b.re - a.im * b.im, a.re * b.im + a.im * b.re};
}

RaderFFT8::Complex RaderFFT8::cadd(const Complex& a, const Complex& b)
{
    return {a.re + b.re, a.im + b.im};
}

RaderFFT8::Complex RaderFFT8::conj(const Complex& a)
{
    return {a.re, -a.im};
}

/* ---- Configure ---- */

bool RaderFFT8::configure(int N)
{
    if (N < 2) return false;
    m_N = N;
    m_paddedLen = nextPow2(2 * N - 1);
    precomputeChirp();
    m_stats.transformSize = N;
    return true;
}

/* ---- Precompute chirp ---- */

void RaderFFT8::precomputeChirp()
{
    int N = m_N;
    m_chirp.resize(N);

    // Bluestein chirp: a[k] = exp(j * pi * k^2 / N) for k = 0..N-1
    for (int k = 0; k < N; ++k) {
        double phase = M_PI * k * k / N;
        m_chirp[k].re = qCos(phase);
        m_chirp[k].im = qSin(phase);
    }

    // Build padded chirp for convolution: length = m_paddedLen
    // b[k] = a[k] for k = 0..N-1, b[M-N+1+k] = a[N-1-k] for k=1..N-1
    m_chirpPad.resize(m_paddedLen);
    for (int i = 0; i < m_paddedLen; ++i) {
        m_chirpPad[i] = {0.0, 0.0};
    }
    for (int k = 0; k < N; ++k) {
        m_chirpPad[k] = m_chirp[k];
    }
    for (int k = 1; k < N; ++k) {
        m_chirpPad[m_paddedLen - N + k] = m_chirp[N - k];
    }

    // Precompute FFT of padded chirp
    m_chirpFft = m_chirpPad;
    fftPow2(m_chirpFft, false);
}

/* ---- Radix-2 FFT (in-place, DIT) ---- */

void RaderFFT8::fftPow2(QVector<Complex>& x, bool inverse) const
{
    int N = x.size();
    if (N <= 1) return;

    // Bit-reversal permutation
    int bits = 0;
    for (int tmp = N; tmp > 1; tmp >>= 1) bits++;
    for (int i = 0; i < N; ++i) {
        int rev = 0;
        for (int b = 0; b < bits; ++b) {
            if (i & (1 << b)) rev |= (1 << (bits - 1 - b));
        }
        if (i < rev) std::swap(x[i], x[rev]);
    }

    // Butterfly stages
    double dir = inverse ? 1.0 : -1.0;
    for (int len = 2; len <= N; len <<= 1) {
        double angle = dir * 2.0 * M_PI / len;
        Complex wn = {qCos(angle), qSin(angle)};
        for (int i = 0; i < N; i += len) {
            Complex w = {1.0, 0.0};
            for (int j = 0; j < len / 2; ++j) {
                Complex u = x[i + j];
                Complex v = cmul(w, x[i + j + len / 2]);
                x[i + j] = cadd(u, v);
                x[i + j + len / 2] = {u.re - v.re, u.im - v.im};
                w = cmul(w, wn);
            }
        }
    }

    // Scale for inverse
    if (inverse) {
        for (int i = 0; i < N; ++i) {
            x[i].re /= N;
            x[i].im /= N;
        }
    }
}

/* ---- Forward FFT (real input) ---- */

QVector<RaderFFT8::Complex> RaderFFT8::forward(const QVector<double>& input)
{
    QVector<Complex> cx(input.size());
    for (int i = 0; i < input.size(); ++i) {
        cx[i].re = input[i];
        cx[i].im = 0.0;
    }
    return forwardComplex(cx);
}

/* ---- Forward FFT (complex input, Bluestein) ---- */

QVector<RaderFFT8::Complex> RaderFFT8::forwardComplex(const QVector<Complex>& input)
{
    QElapsedTimer timer;
    timer.start();

    int N = m_N;
    QVector<Complex> result(N);

    if (input.size() < N) return result;

    // Bluestein's algorithm:
    // Y[k] = sum_n (x[n] * a[n]) * a[k] * a_conj[k-n]
    // Rewrite as cyclic convolution via FFT

    // Step 1: Multiply input by chirp
    QVector<Complex> xp(m_paddedLen);
    for (int i = 0; i < m_paddedLen; ++i) xp[i] = {0.0, 0.0};
    for (int n = 0; n < N; ++n) {
        xp[n] = cmul(input[n], m_chirp[n]);
    }

    // Step 2: FFT of xp
    fftPow2(xp, false);

    // Step 3: Multiply by precomputed chirp FFT
    for (int i = 0; i < m_paddedLen; ++i) {
        xp[i] = cmul(xp[i], m_chirpFft[i]);
    }

    // Step 4: Inverse FFT
    fftPow2(xp, true);

    // Step 5: Multiply by chirp to get final result
    for (int k = 0; k < N; ++k) {
        result[k] = cmul(xp[k], m_chirp[k]);
    }

    m_stats.numForward++;
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit forwardCompleted(N, timer.elapsed());
    return result;
}

/* ---- Inverse FFT ---- */

QVector<RaderFFT8::Complex> RaderFFT8::inverse(const QVector<Complex>& spectrum)
{
    QElapsedTimer timer;
    timer.start();

    int N = m_N;
    QVector<Complex> result(N);

    // Conjugate input, forward transform, conjugate output, scale
    QVector<Complex> conjInput(N);
    for (int i = 0; i < N; ++i)
        conjInput[i] = conj(spectrum[i]);

    QVector<Complex> fwd = forwardComplex(conjInput);
    for (int i = 0; i < N; ++i) {
        result[i] = conj(fwd[i]);
        result[i].re /= N;
        result[i].im /= N;
    }

    m_stats.numInverse++;
    emit inverseCompleted(N, timer.elapsed());
    return result;
}

/* ---- Reset ---- */

void RaderFFT8::resetStatistics()
{
    m_chirp.clear();
    m_chirpPad.clear();
    m_chirpFft.clear();
    m_N = 0;
    m_paddedLen = 0;
    m_stats = Stats{};
    m_timeSum = 0.0;
}
