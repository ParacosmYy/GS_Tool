/**
 * @file PrimeFactorFFT4.cpp
 * @brief PrimeFactorFFT4 实现
 *
 * 实现素因子FFT：Good-Thomas算法、互质分解、CRT索引映射、小N点DFT。
 */

#include "utils/fft178/PrimeFactorFFT4.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

PrimeFactorFFT4::PrimeFactorFFT4(QObject *parent)
    : QObject(parent)
{
}

PrimeFactorFFT4::~PrimeFactorFFT4() = default;

/* ---- Coprime factorization ---- */

QPair<int, int> PrimeFactorFFT4::factorize(int N) const
{
    /* Find coprime factors n1*n2 = N, gcd(n1,n2) = 1 */
    for (int n1 = 2; n1 * n1 <= N; ++n1) {
        if (N % n1 == 0) {
            int n2 = N / n1;
            /* Check coprimality via Euclidean gcd */
            int a = n1, b = n2;
            while (b > 0) { int t = b; b = a % b; a = t; }
            if (a == 1)
                return {n1, n2};
        }
    }
    /* Prime N: cannot factorize into coprime pair (except 1,N) */
    return {1, N};
}

/* ---- CRT index mapping ---- */

int PrimeFactorFFT4::crtMap(int k, int n1, int n2) const
{
    /* Rader's CRT: k1 = k mod n1, k2 = k mod n2
     * Reconstruct via CRT: k = k1*n2*inv(n2,n1) + k2*n1*inv(n1,n2) mod N */
    int k1 = k % n1;
    int k2 = k % n2;

    /* Compute modular inverse of n2 mod n1 */
    int inv_n2_mod_n1 = 1;
    for (int i = 1; i < n1; ++i)
        if ((n2 * i) % n1 == 1) { inv_n2_mod_n1 = i; break; }

    /* Compute modular inverse of n1 mod n2 */
    int inv_n1_mod_n2 = 1;
    for (int i = 1; i < n2; ++i)
        if ((n1 * i) % n2 == 1) { inv_n1_mod_n2 = i; break; }

    int N = n1 * n2;
    int result = (k1 * n2 * inv_n2_mod_n1 + k2 * n1 * inv_n1_mod_n2) % N;
    if (result < 0) result += N;
    return result;
}

/* ---- Short-N DFT (direct computation) ---- */

void PrimeFactorFFT4::shortDFT(QVector<double>& re, QVector<double>& im,
                                 int N, bool inverse) const
{
    int halfN = re.size() / N;
    if (halfN <= 0) return;

    /* For each block of N elements, compute DFT directly */
    for (int block = 0; block < halfN; ++block) {
        int base = block * N;
        QVector<double> tmpRe(N), tmpIm(N);

        for (int k = 0; k < N; ++k) {
            double sumRe = 0.0, sumIm = 0.0;
            for (int n = 0; n < N; ++n) {
                double angle = 2.0 * M_PI * k * n / N * (inverse ? -1.0 : 1.0);
                double wRe = qCos(angle);
                double wIm = qSin(angle);
                sumRe += re[base + n] * wRe - im[base + n] * wIm;
                sumIm += re[base + n] * wIm + im[base + n] * wRe;
            }
            if (inverse) {
                sumRe /= N;
                sumIm /= N;
            }
            tmpRe[k] = sumRe;
            tmpIm[k] = sumIm;
        }

        for (int k = 0; k < N; ++k) {
            re[base + k] = tmpRe[k];
            im[base + k] = tmpIm[k];
        }
    }
}

/* ---- Transpose 2D matrix (row-major stored in 1D) ---- */

void PrimeFactorFFT4::transpose(QVector<double>& data, int rows, int cols)
{
    QVector<double> tmp(data.size());
    for (int r = 0; r < rows; ++r)
        for (int c = 0; c < cols; ++c)
            tmp[c * rows + r] = data[r * cols + c];
    data = tmp;
}

/* ---- Forward transform ---- */

QPair<QVector<double>, QVector<double>> PrimeFactorFFT4::transform(
    const QVector<double>& inputReal, const QVector<double>& inputImag)
{
    QElapsedTimer timer;
    timer.start();

    int N = inputReal.size();
    if (N == 0) return {{}, {}};

    auto [n1, n2] = factorize(N);

    QVector<double> re = inputReal;
    QVector<double> im = inputImag.isEmpty() ? QVector<double>(N, 0.0) : inputImag;

    if (n1 == 1) {
        /* Prime size: direct DFT */
        n1 = 1; n2 = N;
        QVector<double> outRe(N), outIm(N);
        for (int k = 0; k < N; ++k) {
            double sr = 0.0, si = 0.0;
            for (int n = 0; n < N; ++n) {
                double angle = 2.0 * M_PI * k * n / N;
                sr += re[n] * qCos(angle) - im[n] * qSin(angle);
                si += re[n] * qSin(angle) + im[n] * qCos(angle);
            }
            outRe[k] = sr;
            outIm[k] = si;
        }
        re = outRe;
        im = outIm;
    } else {
        /* Good-Thomas algorithm:
         * 1. CRT input permutation */
        QVector<double> permRe(N), permIm(N);
        for (int k = 0; k < N; ++k) {
            int pk = crtMap(k, n1, n2);
            permRe[pk] = re[k];
            permIm[pk] = im[k];
        }

        /* 2. Row DFTs (length n1) */
        re = permRe; im = permIm;
        shortDFT(re, im, n1, false);

        /* 3. Transpose n2 x n1 -> n1 x n2 */
        transpose(re, n2, n1);
        transpose(im, n2, n1);

        /* 4. Column DFTs (length n2) */
        shortDFT(re, im, n2, false);

        /* 5. Transpose back */
        transpose(re, n1, n2);
        transpose(im, n1, n2);

        /* 6. CRT output permutation */
        QVector<double> outRe(N), outIm(N);
        for (int k = 0; k < N; ++k) {
            int pk = crtMap(k, n1, n2);
            outRe[k] = re[pk];
            outIm[k] = im[pk];
        }
        re = outRe; im = outIm;
    }

    m_stats.totalTransforms++;
    m_stats.inputSize = N;
    m_stats.factor1 = n1;
    m_stats.factor2 = n2;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalTransforms;

    emit transformCompleted(N, n1, n2);
    return {re, im};
}

/* ---- Inverse transform ---- */

QPair<QVector<double>, QVector<double>> PrimeFactorFFT4::inverseTransform(
    const QVector<double>& re, const QVector<double>& im)
{
    int N = re.size();
    if (N == 0) return {{}, {}};

    /* Conjugate, forward transform, conjugate, scale */
    QVector<double> conjIm(N);
    for (int i = 0; i < N; ++i) conjIm[i] = -im[i];

    auto result = transform(re, conjIm);

    for (int i = 0; i < N; ++i) {
        result.first[i] /= N;
        result.second[i] = -result.second[i] / N;
    }
    return result;
}

void PrimeFactorFFT4::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
