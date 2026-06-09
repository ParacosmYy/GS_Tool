/**
 * @file MixedRadixFFT9.cpp
 * @brief MixedRadixFFT9 实现
 *
 * 实现混合基数FFT：运行时因式分解与旋转因子递归通用复合N变换。
 */

#include "utils/fft261/MixedRadixFFT9.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

MixedRadixFFT9::MixedRadixFFT9(QObject *parent)
    : QObject(parent) {}
MixedRadixFFT9::~MixedRadixFFT9() = default;

/* ---- Factorize N into small prime factors ---- */

QVector<int> MixedRadixFFT9::factorize(int n) const
{
    QVector<int> facs;
    int remaining = n;
    // Extract factors 2, 3, 5 first (common FFT radices)
    for (int p : {2, 3, 5, 7, 11, 13}) {
        while (remaining % p == 0) {
            facs.append(p);
            remaining /= p;
        }
    }
    // Any remaining prime factor
    int d = 17;
    while (remaining > 1) {
        while (remaining % d == 0) {
            facs.append(d);
            remaining /= d;
        }
        d += 2;
        if (d * d > remaining) {
            if (remaining > 1) facs.append(remaining);
            break;
        }
    }
    // Sort ascending for better cache behavior
    std::sort(facs.begin(), facs.end());
    return facs;
}

/* ---- Twiddle factor ---- */

void MixedRadixFFT9::twiddle(int k, int n, double& re, double& im, bool inverse) const
{
    double angle = (inverse ? 1.0 : -1.0) * 2.0 * M_PI * k / n;
    re = qCos(angle);
    im = qSin(angle);
}

/* ---- Digit-reversal permutation ---- */

void MixedRadixFFT9::digitReverse(QVector<double>& real, QVector<double>& imag,
                                    int n, const QVector<int>& factors) const
{
    int numFacs = factors.size();
    QVector<int> digits(numFacs, 0);
    QVector<int> strides(numFacs, 1);
    // Compute strides (cumulative product of factors)
    for (int i = numFacs - 2; i >= 0; --i)
        strides[i] = strides[i + 1] * factors[i + 1];

    for (int i = 0; i < n; ++i) {
        // Compute digit-reversed index
        int rev = 0;
        int temp = i;
        for (int j = numFacs - 1; j >= 0; --j) {
            digits[j] = temp % factors[j];
            temp /= factors[j];
        }
        for (int j = 0; j < numFacs; ++j)
            rev += digits[j] * strides[j];
        if (i < rev) {
            std::swap(real[i], real[rev]);
            std::swap(imag[i], imag[rev]);
        }
    }
}

/* ---- Recursive mixed-radix DFT ---- */

void MixedRadixFFT9::mixedRadixDFT(QVector<double>& real, QVector<double>& imag,
                                      int n, int stride, bool inverse) const
{
    if (n == 1) return;
    if (n == 2) {
        // Radix-2 butterfly
        double ar = real[0], ai = imag[0];
        double br = real[stride], bi = imag[stride];
        real[0] = ar + br;
        imag[0] = ai + bi;
        real[stride] = ar - br;
        imag[stride] = ai - bi;
        return;
    }

    // Find a factor to split
    int r = 2;
    for (int p : {2, 3, 5, 7}) {
        if (n % p == 0) { r = p; break; }
    }
    int m = n / r;

    // DFT on each of the r groups of size m
    for (int j = 0; j < r; ++j) {
        // Gather j-th elements into contiguous buffer
        QVector<double> subReal(m), subImag(m);
        for (int i = 0; i < m; ++i) {
            subReal[i] = real[(j + i * r) * stride];
            subImag[i] = imag[(j + i * r) * stride];
        }
        // Work in-place on sub-array
        QVector<double> tmpReal = subReal, tmpImag = subImag;
        mixedRadixDFT(tmpReal, tmpImag, m, 1, inverse);
        // Apply twiddle and scatter
        for (int i = 0; i < m; ++i) {
            double twRe, twIm;
            twiddle(j * i, n, twRe, twIm, inverse);
            double re = tmpReal[i] * twRe - tmpImag[i] * twIm;
            double im = tmpReal[i] * twIm + tmpImag[i] * twRe;
            real[(j + i * r) * stride] = re;
            imag[(j + i * r) * stride] = im;
        }
    }

    // Combine using DFT of size r along the outer dimension
    QVector<double> outReal(n), outImag(n);
    for (int k = 0; k < m; ++k) {
        for (int j = 0; j < r; ++j) {
            double sumRe = 0.0, sumIm = 0.0;
            for (int s = 0; s < r; ++s) {
                double twRe, twIm;
                twiddle(j * s, r, twRe, twIm, inverse);
                double vr = real[(s + k * r) * stride];
                double vi = imag[(s + k * r) * stride];
                sumRe += vr * twRe - vi * twIm;
                sumIm += vr * twIm + vi * twRe;
            }
            outReal[j * m + k] = sumRe;
            outImag[j * m + k] = sumIm;
        }
    }
    // Scatter back
    for (int i = 0; i < n; ++i) {
        real[i * stride] = outReal[i];
        imag[i * stride] = outImag[i];
    }
}

/* ---- Forward transform ---- */

QVector<double> MixedRadixFFT9::forward(const QVector<double>& real, const QVector<double>& imag)
{
    QElapsedTimer timer;
    timer.start();

    int n = real.size();
    if (n != imag.size() || n == 0) return {};
    m_n = n;
    m_factors = factorize(n);

    QVector<double> re = real, im = imag;
    digitReverse(re, im, n, m_factors);
    mixedRadixDFT(re, im, n, 1, false);

    // Interleave output
    QVector<double> result(2 * n);
    for (int i = 0; i < n; ++i) {
        result[2 * i] = re[i];
        result[2 * i + 1] = im[i];
    }

    double elapsed = timer.elapsed();
    m_stats.transformSize = n;
    m_stats.numTransforms++;
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit transformCompleted(n, false, elapsed);
    return result;
}

/* ---- Inverse transform ---- */

QVector<double> MixedRadixFFT9::inverse(const QVector<double>& real, const QVector<double>& imag)
{
    QElapsedTimer timer;
    timer.start();

    int n = real.size();
    if (n != imag.size() || n == 0) return {};
    m_n = n;
    m_factors = factorize(n);

    QVector<double> re = real, im = imag;
    digitReverse(re, im, n, m_factors);
    mixedRadixDFT(re, im, n, 1, true);
    // Scale by 1/N
    for (int i = 0; i < n; ++i) { re[i] /= n; im[i] /= n; }

    QVector<double> result(2 * n);
    for (int i = 0; i < n; ++i) {
        result[2 * i] = re[i];
        result[2 * i + 1] = im[i];
    }

    double elapsed = timer.elapsed();
    m_stats.numInverseTransforms++;
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit transformCompleted(n, true, elapsed);
    return result;
}

/* ---- Accessors ---- */

QVector<int> MixedRadixFFT9::factors() const { return m_factors; }

/* ---- Reset ---- */

void MixedRadixFFT9::resetStatistics()
{
    m_factors.clear();
    m_n = 0;
    m_stats = Stats{};
    m_timeSum = 0.0;
}
