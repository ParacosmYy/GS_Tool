/**
 * @file RaderFFT4.cpp
 * @brief RaderFFT4 实现
 *
 * 实现Rader FFT：素数长度DFT转循环卷积、原根生成、Chirp-z转换。
 */

#include "utils/fft179/RaderFFT4.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

RaderFFT4::RaderFFT4(QObject *parent) : QObject(parent) {}
RaderFFT4::~RaderFFT4() = default;

/* ---- Primality test (trial division) ---- */

bool RaderFFT4::isPrime(int n) const
{
    if (n < 2) return false;
    if (n < 4) return true;
    if (n % 2 == 0 || n % 3 == 0) return false;
    for (int i = 5; i * i <= n; i += 6)
        if (n % i == 0 || n % (i + 2) == 0) return false;
    return true;
}

/* ---- Find primitive root modulo N ---- */

int RaderFFT4::primitiveRoot(int N) const
{
    if (N <= 2) return 1;
    /* Factor N-1 */
    int phi = N - 1;
    QVector<int> factors;
    int tmp = phi;
    for (int p = 2; p * p <= tmp; ++p) {
        if (tmp % p == 0) {
            factors.append(p);
            while (tmp % p == 0) tmp /= p;
        }
    }
    if (tmp > 1) factors.append(tmp);

    /* Test candidates g where g^((phi/f) mod N) != 1 for all prime factors f */
    for (int g = 2; g < N; ++g) {
        bool ok = true;
        for (int f : factors) {
            /* Fast modular exponentiation */
            qint64 result = 1;
            qint64 base = g;
            int exp = phi / f;
            while (exp > 0) {
                if (exp & 1) result = (result * base) % N;
                base = (base * base) % N;
                exp >>= 1;
            }
            if (result == 1) { ok = false; break; }
        }
        if (ok) return g;
    }
    return 1;
}

/* ---- Generate permutation from primitive root powers ---- */

QVector<int> RaderFFT4::generatePermutation(int N) const
{
    int g = primitiveRoot(N);
    QVector<int> perm(N - 1);
    qint64 val = 1;
    for (int i = 0; i < N - 1; ++i) {
        perm[i] = static_cast<int>(val);
        val = (val * g) % N;
    }
    return perm;
}

/* ---- Circular convolution (direct method) ---- */

void RaderFFT4::circularConvolve(const QVector<double>& aRe,
                                   const QVector<double>& aIm,
                                   const QVector<double>& bRe,
                                   const QVector<double>& bIm,
                                   QVector<double>& outRe,
                                   QVector<double>& outIm) const
{
    int M = aRe.size();
    outRe.resize(M);
    outIm.resize(M);

    for (int n = 0; n < M; ++n) {
        double sr = 0.0, si = 0.0;
        for (int k = 0; k < M; ++k) {
            int idx = (n - k + M) % M;
            sr += aRe[k] * bRe[idx] - aIm[k] * bIm[idx];
            si += aRe[k] * bIm[idx] + aIm[k] * bRe[idx];
        }
        outRe[n] = sr;
        outIm[n] = si;
    }
}

/* ---- Direct DFT for small N ---- */

void RaderFFT4::directDFT(const QVector<double>& inRe,
                            const QVector<double>& inIm,
                            QVector<double>& outRe, QVector<double>& outIm,
                            bool inverse) const
{
    int N = inRe.size();
    outRe.resize(N);
    outIm.resize(N);
    double sign = inverse ? 1.0 : -1.0;

    for (int k = 0; k < N; ++k) {
        double sr = 0.0, si = 0.0;
        for (int n = 0; n < N; ++n) {
            double angle = sign * 2.0 * M_PI * k * n / N;
            sr += inRe[n] * qCos(angle) - inIm[n] * qSin(angle);
            si += inRe[n] * qSin(angle) + inIm[n] * qCos(angle);
        }
        if (inverse) { sr /= N; si /= N; }
        outRe[k] = sr;
        outIm[k] = si;
    }
}

/* ---- Forward Rader FFT ---- */

QPair<QVector<double>, QVector<double>> RaderFFT4::transform(
    const QVector<double>& inputReal, const QVector<double>& inputImag)
{
    QElapsedTimer timer;
    timer.start();

    int N = inputReal.size();
    if (N == 0) return {{}, {}};

    QVector<double> re = inputReal;
    QVector<double> im = inputImag.isEmpty() ? QVector<double>(N, 0.0) : inputImag;
    QVector<double> outRe(N), outIm(N);

    if (!isPrime(N) || N <= 4) {
        /* Fallback: direct DFT for non-prime or tiny N */
        directDFT(re, im, outRe, outIm, false);
    } else {
        int g = primitiveRoot(N);
        auto perm = generatePermutation(N);

        /* DC component: X[0] = sum of input */
        outRe[0] = 0.0;
        outIm[0] = 0.0;
        for (int i = 0; i < N; ++i) {
            outRe[0] += re[i];
            outIm[0] += im[i];
        }

        /* Build chirp sequences: a[m] = x[g^m] * W_N^{g^m}
         * b[m] = W_N^{g^{-m}} (conjugate chirp) */
        int M = N - 1;
        QVector<double> aRe(M), aIm(M), bRe(M), bIm(M);

        for (int m = 0; m < M; ++m) {
            /* Input permuted by primitive root */
            int idx = perm[m];
            double angle = -2.0 * M_PI * idx / N;
            /* W_N^idx * x[idx] */
            aRe[m] = re[idx] * qCos(angle) - im[idx] * qSin(angle);
            aIm[m] = re[idx] * qSin(angle) + im[idx] * qCos(angle);

            /* Chirp: W_N^{g^{-m}} */
            /* g^{-m} mod N: inverse permutation lookup */
            int negIdx = perm[(M - m) % M];
            double bAngle = -2.0 * M_PI * negIdx / N;
            bRe[m] = qCos(bAngle);
            bIm[m] = qSin(bAngle);
        }

        /* Circular convolution of a and b */
        QVector<double> convRe, convIm;
        circularConvolve(aRe, aIm, bRe, bIm, convRe, convIm);

        /* Extract output from convolution */
        for (int m = 0; m < M; ++m) {
            int k = perm[m];
            outRe[k] = convRe[m];
            outIm[k] = convIm[m];
        }
    }

    m_stats.totalTransforms++;
    m_stats.primeSize = N;
    m_stats.primitiveRoot = isPrime(N) ? primitiveRoot(N) : 0;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalTransforms;

    emit transformCompleted(N, m_stats.primitiveRoot);
    return {outRe, outIm};
}

/* ---- Inverse transform ---- */

QPair<QVector<double>, QVector<double>> RaderFFT4::inverseTransform(
    const QVector<double>& re, const QVector<double>& im)
{
    int N = re.size();
    QVector<double> conjIm(N);
    for (int i = 0; i < N; ++i) conjIm[i] = -im[i];

    auto result = transform(re, conjIm);
    for (int i = 0; i < N; ++i) {
        result.first[i] /= N;
        result.second[i] = -result.second[i] / N;
    }
    return result;
}

/* ---- Reset ---- */

void RaderFFT4::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
