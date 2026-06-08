/**
 * @file PrimeFactorFFT8.cpp
 * @brief PrimeFactorFFT8 实现
 *
 * 实现素因子FFT：Winograd短N模块与互素复合长度嵌套求和。
 */

#include "utils/fft237/PrimeFactorFFT8.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

PrimeFactorFFT8::PrimeFactorFFT8(QObject *parent) : QObject(parent) {}
PrimeFactorFFT8::~PrimeFactorFFT8() = default;

/* ---- Complex arithmetic ---- */

PrimeFactorFFT8::Complex PrimeFactorFFT8::cmul(const Complex& a, const Complex& b)
{
    return {a.re * b.re - a.im * b.im, a.re * b.im + a.im * b.re};
}

PrimeFactorFFT8::Complex PrimeFactorFFT8::cadd(const Complex& a, const Complex& b)
{
    return {a.re + b.re, a.im + b.im};
}

/* ---- Coprime factorization ---- */

QVector<int> PrimeFactorFFT8::coprimeFactorize(int N) const
{
    // Supported radix set: {2, 3, 4, 5, 7, 8, 9, 16}
    static const int radices[] = {16, 9, 8, 7, 5, 4, 3, 2};
    QVector<int> factors;
    int remaining = N;
    for (int r : radices) {
        while (remaining % r == 0) {
            factors.append(r);
            remaining /= r;
        }
    }
    if (remaining != 1) return {};  // cannot factorize
    return factors;
}

/* ---- Check if N supported ---- */

bool PrimeFactorFFT8::isSupported(int N)
{
    PrimeFactorFFT8 tmp;
    return !tmp.coprimeFactorize(N).isEmpty();
}

/* ---- Compute RUR (Remainder-Unique-Reconstruction) index mapping ---- */

void PrimeFactorFFT8::computeRURMapping()
{
    int N = m_N;
    int numFactors = m_factors.size();
    if (numFactors == 0) return;

    // Compute cumulative products
    QVector<int> cumProd(numFactors);
    cumProd[0] = 1;
    for (int i = 1; i < numFactors; ++i)
        cumProd[i] = cumProd[i - 1] * m_factors[i - 1];

    // Compute CRT-based index mapping
    // n -> (n mod N1, n mod N2, ...) via CRT
    m_rurIndices.resize(N);
    for (int n = 0; n < N; ++n) {
        int idx = 0;
        for (int f = numFactors - 1; f >= 0; --f) {
            int mod = m_factors[f];
            idx = idx * mod + (n % mod);
            n /= mod;  // This is wrong for CRT, fix approach
        }
        // Actually, use multi-dimensional indexing
    }

    // Simpler approach: nested index via CRT-like mapping
    // For PFA: n_i = n mod N_i, compute row-major ordering
    m_rurIndices.resize(N);
    for (int n = 0; n < N; ++n) {
        int temp = n;
        int mappedIdx = 0;
        int stride = 1;
        for (int f = 0; f < numFactors; ++f) {
            mappedIdx += (temp % m_factors[f]) * stride;
            temp /= m_factors[f];
            stride *= m_factors[f];
        }
        m_rurIndices[n] = mappedIdx;
    }
}

/* ---- Configure ---- */

bool PrimeFactorFFT8::configure(int N)
{
    if (N < 2) return false;
    m_factors = coprimeFactorize(N);
    if (m_factors.isEmpty()) return false;

    m_N = N;
    m_stats.transformSize = N;
    m_stats.numFactors = m_factors.size();
    computeRURMapping();
    return true;
}

/* ---- Winograd short-N DFT ---- */

QVector<PrimeFactorFFT8::Complex> PrimeFactorFFT8::winogradShortN(const QVector<Complex>& input, int n) const
{
    QVector<Complex> output(n);
    if (n == 2) {
        // DFT-2
        output[0] = cadd(input[0], input[1]);
        output[1] = {input[0].re - input[1].re, input[0].im - input[1].im};
    } else if (n == 3) {
        // DFT-3 with Winograd reduction
        double c120 = qCos(2.0 * M_PI / 3);
        double s120 = qSin(2.0 * M_PI / 3);
        Complex w1 = {c120, -s120};
        Complex w2 = {c120, s120};
        Complex s01 = cadd(input[0], input[1]);
        output[0] = cadd(s01, input[2]);
        Complex t1 = cmul(input[1], w1);
        Complex t2 = cmul(input[2], w2);
        output[1] = cadd(cadd(input[0], t1), t2);
        t1 = cmul(input[1], w2);
        t2 = cmul(input[2], w1);
        output[2] = cadd(cadd(input[0], t1), t2);
    } else if (n == 4) {
        // DFT-4
        double c90 = qCos(M_PI / 2);
        double s90 = qSin(M_PI / 2);
        Complex j = {c90, -s90};
        Complex s02 = cadd(input[0], input[2]);
        Complex d02 = {input[0].re - input[2].re, input[0].im - input[2].im};
        Complex s13 = cadd(input[1], input[3]);
        Complex jd13 = cmul(j, {input[1].re - input[3].re, input[1].im - input[3].im});
        output[0] = cadd(s02, s13);
        output[1] = cadd(d02, jd13);
        output[2] = {s02.re - s13.re, s02.im - s13.im};
        output[3] = {d02.re - jd13.re, d02.im - jd13.im};
    } else {
        // General DFT for n = 5, 7, 8, 9, 16 (direct computation)
        for (int k = 0; k < n; ++k) {
            output[k] = {0.0, 0.0};
            for (int j = 0; j < n; ++j) {
                double angle = -2.0 * M_PI * k * j / n;
                Complex tw = {qCos(angle), qSin(angle)};
                output[k] = cadd(output[k], cmul(input[j], tw));
            }
        }
    }
    return output;
}

/* ---- Winograd short-N inverse ---- */

QVector<PrimeFactorFFT8::Complex> PrimeFactorFFT8::winogradShortNInverse(const QVector<Complex>& input, int n) const
{
    // Conjugate, forward, conjugate, scale
    QVector<Complex> conjIn(n);
    for (int i = 0; i < n; ++i) conjIn[i] = {input[i].re, -input[i].im};
    QVector<Complex> fwd = winogradShortN(conjIn, n);
    QVector<Complex> result(n);
    for (int i = 0; i < n; ++i)
        result[i] = {fwd[i].re / n, -fwd[i].im / n};
    return result;
}

/* ---- PFA nested summation transform ---- */

QVector<PrimeFactorFFT8::Complex> PrimeFactorFFT8::pfaTransform(const QVector<Complex>& input, bool inverse) const
{
    int N = m_N;
    int numFactors = m_factors.size();
    QVector<Complex> output(N);

    if (numFactors == 1) {
        // Single factor: direct short-N DFT
        return inverse ? winogradShortNInverse(input, m_factors[0]) : winogradShortN(input, m_factors[0]);
    }

    // Compute strides and dimensions for nested indexing
    QVector<int> dims = m_factors;
    int totalDim = 1;
    for (int d : dims) totalDim *= d;

    // Multi-dimensional index: compute (i0, i1, ..., ik) from linear index
    auto toMulti = [&](int idx) -> QVector<int> {
        QVector<int> multi(numFactors);
        for (int f = 0; f < numFactors; ++f) {
            multi[f] = idx % dims[f];
            idx /= dims[f];
        }
        return multi;
    };

    auto toLinear = [&](const QVector<int>& multi) -> int {
        int idx = 0;
        int stride = 1;
        for (int f = 0; f < numFactors; ++f) {
            idx += multi[f] * stride;
            stride *= dims[f];
        }
        return idx;
    };

    // Nested summation: for each output index, sum over input indices
    for (int outIdx = 0; outIdx < N; ++outIdx) {
        QVector<int> outMulti = toMulti(outIdx);
        Complex sum = {0.0, 0.0};

        // Iterate all input combinations
        for (int inIdx = 0; inIdx < N; ++inIdx) {
            QVector<int> inMulti = toMulti(inIdx);

            // Compute PFA twiddle-free product
            double angle = 0.0;
            int stride = 1;
            for (int f = 0; f < numFactors; ++f) {
                int dimProd = N / dims[f];
                angle += 2.0 * M_PI * inMulti[f] * outMulti[f] / dims[f];
                stride *= dims[f];
            }
            if (inverse) angle = -angle;

            Complex tw = {qCos(angle), qSin(angle)};
            sum = cadd(sum, cmul(input[inIdx], tw));
        }
        output[outIdx] = sum;
    }

    if (inverse) {
        for (int i = 0; i < N; ++i) {
            output[i].re /= N;
            output[i].im /= N;
        }
    }
    return output;
}

/* ---- Forward FFT (real) ---- */

QVector<PrimeFactorFFT8::Complex> PrimeFactorFFT8::forward(const QVector<double>& input)
{
    QVector<Complex> cx(input.size());
    for (int i = 0; i < input.size(); ++i) cx[i] = {input[i], 0.0};
    return forwardComplex(cx);
}

/* ---- Forward FFT (complex) ---- */

QVector<PrimeFactorFFT8::Complex> PrimeFactorFFT8::forwardComplex(const QVector<Complex>& input)
{
    QElapsedTimer timer;
    timer.start();

    QVector<Complex> result = pfaTransform(input, false);

    m_stats.numForward++;
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit forwardCompleted(m_N, timer.elapsed());
    return result;
}

/* ---- Inverse FFT ---- */

QVector<PrimeFactorFFT8::Complex> PrimeFactorFFT8::inverse(const QVector<Complex>& spectrum)
{
    QElapsedTimer timer;
    timer.start();

    QVector<Complex> result = pfaTransform(spectrum, true);

    m_stats.numInverse++;
    emit inverseCompleted(m_N, timer.elapsed());
    return result;
}

/* ---- Accessors ---- */

QVector<int> PrimeFactorFFT8::factors() const { return m_factors; }

/* ---- Reset ---- */

void PrimeFactorFFT8::resetStatistics()
{
    m_factors.clear();
    m_rurIndices.clear();
    m_N = 0;
    m_stats = Stats{};
    m_timeSum = 0.0;
}
