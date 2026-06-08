/**
 * @file MixedRadixFFT7.cpp
 * @brief MixedRadixFFT7 实现
 *
 * 实现混合基数FFT：素因子分解树运行时选择与缓存感知旋转因子预计算。
 */

#include "utils/fft233/MixedRadixFFT7.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

MixedRadixFFT7::MixedRadixFFT7(QObject *parent) : QObject(parent) {}
MixedRadixFFT7::~MixedRadixFFT7() = default;

/* ---- Prime factorization ---- */

QVector<int> MixedRadixFFT7::primeFactorize(int n) const
{
    QVector<int> factors;
    // Extract 2s (largest composite for efficiency)
    while (n % 2 == 0) { factors.append(2); n /= 2; }
    // Extract 3s
    while (n % 3 == 0) { factors.append(3); n /= 3; }
    // Extract 5s
    while (n % 5 == 0) { factors.append(5); n /= 5; }
    // Extract 7s
    while (n % 7 == 0) { factors.append(7); n /= 7; }
    // Remaining primes (trial division up to sqrt)
    for (int p = 11; p * p <= n; p += 2) {
        while (n % p == 0) { factors.append(p); n /= p; }
    }
    if (n > 1) factors.append(n);
    // Sort descending for better cache behavior in outer stages
    std::sort(factors.begin(), factors.end(), std::greater<int>());
    return factors;
}

/* ---- Configure ---- */

bool MixedRadixFFT7::configure(int n)
{
    if (n < 2) return false;
    m_n = n;
    m_factors = primeFactorize(n);
    m_stats.transformSize = n;
    m_stats.numStages = m_factors.size();
    precomputeTwiddles();
    return true;
}

/* ---- Cache-aware twiddle precomputation ---- */

void MixedRadixFFT7::precomputeTwiddles()
{
    int n = m_n;
    m_twReal.resize(n);
    m_twImag.resize(n);
    m_twCacheTag.resize(n);
    m_stats.twiddleCacheHits = 0;
    m_stats.twiddleCacheMisses = 0;

    // Compute all twiddle factors W_N^k = exp(-2*pi*j*k/N)
    for (int k = 0; k < n; ++k) {
        double angle = -2.0 * M_PI * k / n;
        m_twReal[k] = qCos(angle);
        m_twImag[k] = qSin(angle);
        m_twCacheTag[k] = -1;
    }

    // Tag frequently-accessed twiddles (multiples of factor products)
    int prod = 1;
    int stage = 0;
    for (int f : m_factors) {
        int m = n / (prod * f);
        for (int j = 0; j < m; ++j) {
            for (int i = 0; i < f; ++i) {
                int idx = j * f + i;
                if (idx < n) m_twCacheTag[idx] = stage;
            }
        }
        prod *= f;
        stage++;
    }
}

/* ---- Small-N DFT for prime factors ---- */

void MixedRadixFFT7::smallDFT(double* re, double* im, int n, int stride, int twBase) const
{
    // Direct DFT for small prime sizes
    QVector<double> outRe(n), outIm(n);
    for (int k = 0; k < n; ++k) {
        double sumRe = 0.0, sumIm = 0.0;
        for (int j = 0; j < n; ++j) {
            double angle = -2.0 * M_PI * j * k / n;
            double twR = qCos(angle);
            double twI = qSin(angle);
            double xR = re[j * stride], xI = im[j * stride];
            sumRe += xR * twR - xI * twI;
            sumIm += xR * twI + xI * twR;
        }
        outRe[k] = sumRe;
        outIm[k] = sumIm;
    }
    for (int k = 0; k < n; ++k) {
        re[k * stride] = outRe[k];
        im[k * stride] = outIm[k];
    }
}

/* ---- General radix-r butterfly ---- */

void MixedRadixFFT7::radixButterfly(double* re, double* im, int n, int r, int stride,
                                      int twBase) const
{
    int m = n / r;
    if (r <= 7) {
        // Use small-DFT with twiddle multiply
        QVector<double> subRe(r), subIm(r);
        for (int j = 0; j < m; ++j) {
            for (int i = 0; i < r; ++i) {
                int twIdx = (j + twBase) * i;
                twIdx = twIdx % m_n;
                double twR = m_twReal[twIdx];
                double twI = m_twImag[twIdx];
                double xR = re[j + i * m * stride];
                double xI = im[j + i * m * stride];
                subRe[i] = xR * twR - xI * twI;
                subIm[i] = xR * twI + xI * twR;
            }
            // In-place small DFT
            double savedRe[8], savedIm[8];
            for (int i = 0; i < r; ++i) {
                savedRe[i] = re[j + i * m * stride];
                savedIm[i] = im[j + i * m * stride];
            }

            // Compute DFT of twiddle-multiplied values
            for (int k = 0; k < r; ++k) {
                double sumRe = 0.0, sumIm = 0.0;
                for (int i = 0; i < r; ++i) {
                    double angle = -2.0 * M_PI * i * k / r;
                    double cR = qCos(angle), cI = qSin(angle);
                    sumRe += subRe[i] * cR - subIm[i] * cI;
                    sumIm += subRe[i] * cI + subIm[i] * cR;
                }
                re[j + k * m * stride] = sumRe;
                im[j + k * m * stride] = sumIm;
            }
        }
    } else {
        // Fallback: smallDFT with twiddles
        smallDFT(re, im, r, stride, twBase);
    }
}

/* ---- Digit-reverse permutation ---- */

void MixedRadixFFT7::digitReverse(double* re, double* im) const
{
    int n = m_n;
    QVector<int> revIdx(n, 0);
    int prod = 1;
    for (int f : m_factors) {
        for (int i = 0; i < n; ++i)
            revIdx[i] = revIdx[i] * f + (i / prod) % f;
        prod *= f;
    }
    for (int i = 0; i < n; ++i) {
        if (i < revIdx[i]) {
            std::swap(re[i], re[revIdx[i]]);
            std::swap(im[i], im[revIdx[i]]);
        }
    }
}

/* ---- Forward transform ---- */

QVector<double> MixedRadixFFT7::forward(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    int n = m_n;
    QVector<double> re(n, 0.0), im(n, 0.0);
    for (int i = 0; i < qMin(input.size() / 2, n); ++i) {
        re[i] = input[2 * i];
        im[i] = input[2 * i + 1];
    }

    // DIT stages
    int stride = 1;
    int currentN = 1;
    int twOff = 0;
    for (int f : m_factors) {
        currentN *= f;
        radixButterfly(re.data(), im.data(), currentN, f, stride, twOff);
        stride *= f;
    }

    digitReverse(re.data(), im.data());

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

QVector<double> MixedRadixFFT7::inverse(const QVector<double>& input)
{
    int n = m_n;
    QVector<double> conj(2 * n);
    for (int i = 0; i < n; ++i) {
        conj[2 * i] = input[2 * i];
        conj[2 * i + 1] = -input[2 * i + 1];
    }
    QVector<double> result = forward(conj);
    for (int i = 0; i < n; ++i) {
        result[2 * i] /= n;
        result[2 * i + 1] = -result[2 * i + 1] / n;
    }
    return result;
}

/* ---- Factor tree ---- */

QVector<int> MixedRadixFFT7::factorTree() const { return m_factors; }

/* ---- Reset ---- */

void MixedRadixFFT7::resetStatistics()
{
    m_factors.clear();
    m_twReal.clear();
    m_twImag.clear();
    m_twCacheTag.clear();
    m_n = 0;
    m_stats = Stats{};
    m_timeSum = 0.0;
}
