/**
 * @file HexFFT7.cpp
 * @brief HexFFT7 实现
 *
 * 实现六角FFT：radix-3/6混合分解与旋转因子共享。
 */

#include "utils/fft232/HexFFT7.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

HexFFT7::HexFFT7(QObject *parent) : QObject(parent) {}
HexFFT7::~HexFFT7() = default;

/* ---- Factorize into 3s and 6s ---- */

QVector<int> HexFFT7::factorize6k(int n) const
{
    QVector<int> factors;
    while (n % 6 == 0) { factors.append(6); n /= 6; }
    while (n % 3 == 0) { factors.append(3); n /= 3; }
    // Handle remaining factor (should be 1 for valid 6k)
    if (n > 1) factors.append(n);
    return factors;
}

/* ---- Configure ---- */

bool HexFFT7::configure(int n)
{
    if (n < 6 || n % 6 != 0) return false;

    m_n = n;
    m_factors = factorize6k(n);
    computeTwiddleFactors();

    m_stats.transformSize = n;
    m_stats.numRadix3 = 0;
    m_stats.numRadix6 = 0;
    for (int f : m_factors) {
        if (f == 3) m_stats.numRadix3++;
        else if (f == 6) m_stats.numRadix6++;
    }
    return true;
}

/* ---- Compute twiddle factors with sharing ---- */

void HexFFT7::computeTwiddleFactors()
{
    int totalTwiddle = m_n;
    m_twiddleReal.resize(totalTwiddle);
    m_twiddleImag.resize(totalTwiddle);
    m_sharedTwiddleReal.resize(totalTwiddle);
    m_sharedTwiddleImag.resize(totalTwiddle);

    for (int k = 0; k < totalTwiddle; ++k) {
        double angle = -2.0 * M_PI * k / m_n;
        twiddle(angle, m_twiddleReal[k], m_twiddleImag[k]);
    }

    // Shared bank: twiddles for radix-3 and radix-6 overlap at multiples of N/6
    m_stats.numTwiddleShared = 0;
    for (int k = 0; k < totalTwiddle; ++k) {
        if (k % 6 == 0 || k % 3 == 0) {
            double angle = -2.0 * M_PI * k / m_n;
            twiddle(angle, m_sharedTwiddleReal[k], m_sharedTwiddleImag[k]);
            m_stats.numTwiddleShared++;
        } else {
            m_sharedTwiddleReal[k] = m_twiddleReal[k];
            m_sharedTwiddleImag[k] = m_twiddleImag[k];
        }
    }
}

/* ---- Twiddle helper ---- */

void HexFFT7::twiddle(double angle, double& cosVal, double& sinVal)
{
    cosVal = qCos(angle);
    sinVal = qSin(angle);
}

/* ---- Radix-3 butterfly ---- */

void HexFFT7::radix3Butterfly(double* re, double* im, int stride, int m,
                                const double* twRe, const double* twIm) const
{
    // 3-point DFT with twiddle multiplication
    double re0 = re[0], re1 = re[stride], re2 = re[2 * stride];
    double im0 = im[0], im1 = im[stride], im2 = im[2 * stride];

    // Twiddle multiply
    double tw1R = (twRe != nullptr) ? twRe[m] : 1.0;
    double tw1I = (twIm != nullptr) ? twIm[m] : 0.0;
    double tw2R = (twRe != nullptr) ? twRe[2 * m] : 1.0;
    double tw2I = (twIm != nullptr) ? twIm[2 * m] : 0.0;

    double t1R = re1 * tw1R - im1 * tw1I;
    double t1I = re1 * tw1I + im1 * tw1R;
    double t2R = re2 * tw2R - im2 * tw2I;
    double t2I = re2 * tw2I + im2 * tw2R;

    // 3-point DFT: use exp(-2pi*j/3) = -0.5 - j*sqrt(3)/2
    double c1 = -0.5, s1 = -qSqrt(3.0) / 2.0;
    double c2 = -0.5, s2 = qSqrt(3.0) / 2.0;

    double sumR = t1R + t2R, sumI = t1I + t2I;
    double diff1R = t1R * c1 - t1I * s1 + t2R * c2 - t2I * s2;
    double diff1I = t1R * s1 + t1I * c1 + t2R * s2 + t2I * c2;
    double diff2R = t1R * c2 - t1I * s2 + t2R * c1 - t2I * s1;
    double diff2I = t1R * s2 + t1I * c2 + t2R * s1 + t2I * c1;

    re[0] = re0 + sumR;
    im[0] = im0 + sumI;
    re[stride] = re0 + diff1R;
    im[stride] = im0 + diff1I;
    re[2 * stride] = re0 + diff2R;
    im[2 * stride] = im0 + diff2I;
}

/* ---- Radix-6 butterfly ---- */

void HexFFT7::radix6Butterfly(double* re, double* im, int stride, int m,
                                const double* twRe, const double* twIm) const
{
    // Radix-6 = radix-3 of radix-2
    // First: radix-2 on pairs (0,3), (1,4), (2,5)
    for (int i = 0; i < 3; ++i) {
        int i0 = i * stride, i3 = (i + 3) * stride;
        double twR = (twRe != nullptr) ? twRe[i * m] : 1.0;
        double twI = (twIm != nullptr) ? twIm[i * m] : 0.0;

        double aR = re[i0], aI = im[i0];
        double bR = re[i3] * twR - im[i3] * twI;
        double bI = re[i3] * twI + im[i3] * twR;

        re[i0] = aR + bR; im[i0] = aI + bI;
        re[i3] = aR - bR; im[i3] = aI - bI;
    }

    // Then: radix-3 on (0,1,2) and (3,4,5)
    for (int g = 0; g < 2; ++g) {
        int base = g * 3 * stride;
        double subRe[3], subIm[3];
        for (int i = 0; i < 3; ++i) {
            subRe[i] = re[base + i * stride];
            subIm[i] = im[base + i * stride];
        }

        double c1 = -0.5, s1 = -qSqrt(3.0) / 2.0;
        double c2 = -0.5, s2 = qSqrt(3.0) / 2.0;

        double sumR = subRe[1] + subRe[2], sumI = subIm[1] + subIm[2];
        double d1R = subRe[1] * c1 - subIm[1] * s1 + subRe[2] * c2 - subIm[2] * s2;
        double d1I = subRe[1] * s1 + subIm[1] * c1 + subRe[2] * s2 + subIm[2] * c2;
        double d2R = subRe[1] * c2 - subIm[1] * s2 + subRe[2] * c1 - subIm[2] * s1;
        double d2I = subRe[1] * s2 + subIm[1] * c2 + subRe[2] * s1 + subIm[2] * c1;

        re[base] = subRe[0] + sumR;
        im[base] = subIm[0] + sumI;
        re[base + stride] = subRe[0] + d1R;
        im[base + stride] = subIm[0] + d1I;
        re[base + 2 * stride] = subRe[0] + d2R;
        im[base + 2 * stride] = subIm[0] + d2I;
    }
}

/* ---- Cooley-Tukey stage ---- */

void HexFFT7::ctStage(double* re, double* im, int n, int factor, int stride, int twOff)
{
    int m = n / factor;
    for (int j = 0; j < m; ++j) {
        int off = j * stride * factor;
        if (factor == 3) {
            radix3Butterfly(re + off, im + off, stride, j + twOff,
                            m_sharedTwiddleReal.data(), m_sharedTwiddleImag.data());
        } else if (factor == 6) {
            radix6Butterfly(re + off, im + off, stride, j + twOff,
                            m_sharedTwiddleReal.data(), m_sharedTwiddleImag.data());
        }
    }
}

/* ---- Digit-reverse permutation ---- */

void HexFFT7::digitReverse(double* re, double* im) const
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

QVector<double> HexFFT7::forward(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    int n = m_n;
    QVector<double> re(n, 0.0), im(n, 0.0);

    // De-interleave input
    for (int i = 0; i < qMin(input.size() / 2, n); ++i) {
        re[i] = input[2 * i];
        im[i] = input[2 * i + 1];
    }

    // Cooley-Tukey stages
    int stride = 1;
    int currentN = 1;
    int twOff = 0;
    for (int f : m_factors) {
        currentN *= f;
        ctStage(re.data(), im.data(), currentN, f, stride, twOff);
        stride *= f;
        twOff += currentN / f;
    }

    digitReverse(re.data(), im.data());

    // Interleave output
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

QVector<double> HexFFT7::inverse(const QVector<double>& input)
{
    // Conjugate input
    int n = m_n;
    QVector<double> conj(2 * n);
    for (int i = 0; i < n; ++i) {
        conj[2 * i] = input[2 * i];
        conj[2 * i + 1] = -input[2 * i + 1];
    }

    QVector<double> result = forward(conj);

    // Conjugate and scale
    for (int i = 0; i < n; ++i) {
        result[2 * i] /= n;
        result[2 * i + 1] = -result[2 * i + 1] / n;
    }
    return result;
}

/* ---- Factorization plan ---- */

QVector<int> HexFFT7::factorizationPlan() const { return m_factors; }

/* ---- Reset ---- */

void HexFFT7::resetStatistics()
{
    m_factors.clear();
    m_twiddleReal.clear();
    m_twiddleImag.clear();
    m_sharedTwiddleReal.clear();
    m_sharedTwiddleImag.clear();
    m_n = 0;
    m_stats = Stats{};
    m_timeSum = 0.0;
}
