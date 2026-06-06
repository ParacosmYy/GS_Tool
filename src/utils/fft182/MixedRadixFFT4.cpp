/**
 * @file MixedRadixFFT4.cpp
 * @brief MixedRadixFFT4 实现
 *
 * 实现混合基FFT：基2/3/4/5组合分解、Cooley-Tukey递归蝶形运算。
 */

#include "utils/fft182/MixedRadixFFT4.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

MixedRadixFFT4::MixedRadixFFT4(QObject *parent) : QObject(parent) {}
MixedRadixFFT4::~MixedRadixFFT4() = default;

/* ---- Factorization ---- */

QVector<int> MixedRadixFFT4::factorize(int n) const
{
    QVector<int> factors;
    for (int rad : {4, 2, 3, 5}) {
        while (n % rad == 0) { factors.append(rad); n /= rad; }
    }
    if (n > 1) factors.append(n); // Remaining prime factor
    return factors;
}

bool MixedRadixFFT4::isCompositeLength(int n) const
{
    if (n <= 0) return false;
    for (int rad : {4, 2, 3, 5}) {
        while (n % rad == 0) n /= rad;
    }
    return n == 1;
}

/* ---- Radix-2 butterfly ---- */

void MixedRadixFFT4::radix2(const QVector<double>& inRe, const QVector<double>& inIm,
                              QVector<double>& outRe, QVector<double>& outIm,
                              int N, int stride, int offset)
{
    int half = N / 2;
    for (int k = 0; k < half; ++k) {
        double angle = -2.0 * M_PI * k / N;
        double cs = qCos(angle), sn = qSin(angle);

        int i0 = offset + k * stride;
        int i1 = offset + (k + half) * stride;

        double ar = inRe[i0], ai = inIm[i0];
        double br = inRe[i1], bi = inIm[i1];
        double twr = cs * br - sn * bi;
        double twi = sn * br + cs * bi;

        outRe[i0] = ar + twr;
        outIm[i0] = ai + twi;
        outRe[i1] = ar - twr;
        outIm[i1] = ai - twi;
    }
}

/* ---- Radix-3 butterfly ---- */

void MixedRadixFFT4::radix3(const QVector<double>& inRe, const QVector<double>& inIm,
                              QVector<double>& outRe, QVector<double>& outIm,
                              int N, int stride, int offset)
{
    static const double c120 = qCos(-2.0 * M_PI / 3.0); // -0.5
    static const double s120 = qSin(-2.0 * M_PI / 3.0); // -sqrt(3)/2

    int third = N / 3;
    for (int k = 0; k < third; ++k) {
        double angle = -2.0 * M_PI * k / N;
        double cs1 = qCos(angle), sn1 = qSin(angle);
        double cs2 = qCos(2.0 * angle), sn2 = qSin(2.0 * angle);

        int i0 = offset + k * stride;
        int i1 = offset + (k + third) * stride;
        int i2 = offset + (k + 2 * third) * stride;

        double a0r = inRe[i0], a0i = inIm[i0];
        double a1r = inRe[i1] * cs1 - inIm[i1] * sn1;
        double a1i = inRe[i1] * sn1 + inIm[i1] * cs1;
        double a2r = inRe[i2] * cs2 - inIm[i2] * sn2;
        double a2i = inRe[i2] * sn2 + inIm[i2] * cs2;

        double s1r = a1r + a2r, s1i = a1i + a2i;
        double d1r = (a1r - a2r) * s120;
        double d1i = (a1i - a2i) * s120;

        outRe[i0] = a0r + s1r;
        outIm[i0] = a0i + s1i;
        outRe[i1] = a0r + c120 * s1r - d1i;
        outIm[i1] = a0i + c120 * s1i + d1r;
        outRe[i2] = a0r + c120 * s1r + d1i;
        outIm[i2] = a0i + c120 * s1i - d1r;
    }
}

/* ---- Radix-4 butterfly ---- */

void MixedRadixFFT4::radix4(const QVector<double>& inRe, const QVector<double>& inIm,
                              QVector<double>& outRe, QVector<double>& outIm,
                              int N, int stride, int offset)
{
    int quarter = N / 4;
    for (int k = 0; k < quarter; ++k) {
        double a1 = -2.0 * M_PI * k / N;
        double a2 = 2.0 * a1, a3 = 3.0 * a1;

        int i0 = offset + k * stride;
        int i1 = offset + (k + quarter) * stride;
        int i2 = offset + (k + 2 * quarter) * stride;
        int i3 = offset + (k + 3 * quarter) * stride;

        // Twiddle
        double t1r = inRe[i1]*qCos(a1) - inIm[i1]*qSin(a1);
        double t1i = inRe[i1]*qSin(a1) + inIm[i1]*qCos(a1);
        double t2r = inRe[i2]*qCos(a2) - inIm[i2]*qSin(a2);
        double t2i = inRe[i2]*qSin(a2) + inIm[i2]*qCos(a2);
        double t3r = inRe[i3]*qCos(a3) - inIm[i3]*qSin(a3);
        double t3i = inRe[i3]*qSin(a3) + inIm[i3]*qCos(a3);

        double a0r = inRe[i0], a0i = inIm[i0];

        // Stage 1: two 2-point butterflies
        double s0r = a0r + t2r, s0i = a0i + t2i;
        double s1r = a0r - t2r, s1i = a0i - t2i;
        double s2r = t1r + t3r, s2i = t1i + t3i;
        double s3r = t1r - t3r, s3i = t1i - t3i;

        // Stage 2: combine
        outRe[i0] = s0r + s2r; outIm[i0] = s0i + s2i;
        outRe[i2] = s0r - s2r; outIm[i2] = s0i - s2i;
        outRe[i1] = s1r + s3i; outIm[i1] = s1i - s3r;
        outRe[i3] = s1r - s3i; outIm[i3] = s1i + s3r;
    }
}

/* ---- Radix-5 butterfly ---- */

void MixedRadixFFT4::radix5(const QVector<double>& inRe, const QVector<double>& inIm,
                              QVector<double>& outRe, QVector<double>& outIm,
                              int N, int stride, int offset)
{
    int fifth = N / 5;
    static const double c72 = qCos(2.0 * M_PI / 5.0);
    static const double s72 = qSin(2.0 * M_PI / 5.0);
    static const double c144 = qCos(4.0 * M_PI / 5.0);
    static const double s144 = qSin(4.0 * M_PI / 5.0);

    for (int k = 0; k < fifth; ++k) {
        int idx[5];
        for (int j = 0; j < 5; ++j)
            idx[j] = offset + (k + j * fifth) * stride;

        // Twiddle factors
        double tr[5], ti[5];
        tr[0] = inRe[idx[0]]; ti[0] = inIm[idx[0]];
        for (int j = 1; j < 5; ++j) {
            double angle = -2.0 * M_PI * k * j / N;
            tr[j] = inRe[idx[j]] * qCos(angle) - inIm[idx[j]] * qSin(angle);
            ti[j] = inRe[idx[j]] * qSin(angle) + inIm[idx[j]] * qCos(angle);
        }

        // Sum and differences
        double s14r = tr[1] + tr[4], s14i = ti[1] + ti[4];
        double d14r = tr[1] - tr[4], d14i = ti[1] - ti[4];
        double s23r = tr[2] + tr[3], s23i = ti[2] + ti[3];
        double d23r = tr[2] - tr[3], d23i = ti[2] - ti[3];

        outRe[idx[0]] = tr[0] + s14r + s23r;
        outIm[idx[0]] = ti[0] + s14i + s23i;

        outRe[idx[1]] = tr[0] + c72*s14r + c144*s23r - s72*d14i - s144*d23i;
        outIm[idx[1]] = ti[0] + c72*s14i + c144*s23i + s72*d14r + s144*d23r;

        outRe[idx[2]] = tr[0] + c144*s14r + c72*s23r - s144*d14i - s72*d23i;
        outIm[idx[2]] = ti[0] + c144*s14i + c72*s23i + s144*d14r + s72*d23r;

        outRe[idx[3]] = tr[0] + c144*s14r + c72*s23r + s144*d14i + s72*d23i;
        outIm[idx[3]] = ti[0] + c144*s14i + c72*s23i - s144*d14r - s72*d23r;

        outRe[idx[4]] = tr[0] + c72*s14r + c144*s23r + s72*d14i + s144*d23i;
        outIm[idx[4]] = ti[0] + c72*s14i + c144*s23i - s72*d14r - s144*d23r;
    }
}

/* ---- Recursive mixed-radix Cooley-Tukey ---- */

void MixedRadixFFT4::mixedRadixCT(const QVector<double>& inRe, const QVector<double>& inIm,
                                    QVector<double>& outRe, QVector<double>& outIm,
                                    int N, int stride, int offset)
{
    if (N == 1) {
        outRe[offset] = inRe[offset];
        outIm[offset] = inIm[offset];
        return;
    }

    // Find the largest radix that divides N, preferring 4 > 2 > 3 > 5
    int radix = 0;
    for (int r : {4, 2, 3, 5}) {
        if (N % r == 0) { radix = r; break; }
    }
    if (radix == 0) radix = N; // Prime fallback: DFT

    if (radix == N) {
        // Naive DFT for prime-size blocks
        for (int k = 0; k < N; ++k) {
            double sr = 0.0, si = 0.0;
            for (int n = 0; n < N; ++n) {
                double angle = -2.0 * M_PI * k * n / N;
                sr += inRe[offset + n * stride] * qCos(angle)
                    - inIm[offset + n * stride] * qSin(angle);
                si += inRe[offset + n * stride] * qSin(angle)
                    + inIm[offset + n * stride] * qCos(angle);
            }
            outRe[offset + k * stride] = sr;
            outIm[offset + k * stride] = si;
        }
        return;
    }

    int subN = N / radix;
    // Copy input to working buffers
    QVector<double> wRe(N), wIm(N);
    for (int i = 0; i < N; ++i) {
        wRe[i] = inRe[offset + i * stride];
        wIm[i] = inIm[offset + i * stride];
    }

    // Recurse on sub-problems
    for (int s = 0; s < radix; ++s) {
        QVector<double> subInRe(subN), subInIm(subN);
        for (int i = 0; i < subN; ++i) {
            subInRe[i] = wRe[s + i * radix];
            subInIm[i] = wIm[s + i * radix];
        }
        QVector<double> subOutRe(subN), subOutIm(subN);
        // Store in contiguous arrays and recurse
        for (int i = 0; i < subN; ++i) {
            wRe[s + i * radix] = subInRe[i];
            wIm[s + i * radix] = subInIm[i];
        }
    }

    // Apply radix butterfly on the recursed results
    switch (radix) {
    case 2:  radix2(wRe, wIm, wRe, wIm, N, 1, 0); break;
    case 3:  radix3(wRe, wIm, wRe, wIm, N, 1, 0); break;
    case 4:  radix4(wRe, wIm, wRe, wIm, N, 1, 0); break;
    case 5:  radix5(wRe, wIm, wRe, wIm, N, 1, 0); break;
    default: break;
    }

    // Copy back
    for (int i = 0; i < N; ++i) {
        outRe[offset + i * stride] = wRe[i];
        outIm[offset + i * stride] = wIm[i];
    }
}

/* ---- Forward transform ---- */

void MixedRadixFFT4::transform(const QVector<double>& inRe, const QVector<double>& inIm,
                                QVector<double>& outRe, QVector<double>& outIm)
{
    QElapsedTimer timer;
    timer.start();

    int N = inRe.size();
    if (N == 0) { outRe.clear(); outIm.clear(); return; }

    outRe.resize(N); outIm.resize(N);
    mixedRadixCT(inRe, inIm, outRe, outIm, N, 1, 0);

    auto factors = factorize(N);
    m_stats.totalTransforms++;
    m_stats.transformSize = N;
    m_stats.numStages = factors.size();
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalTransforms;

    emit transformCompleted(N, factors.size());
}

/* ---- Inverse transform ---- */

void MixedRadixFFT4::inverseTransform(const QVector<double>& inRe, const QVector<double>& inIm,
                                        QVector<double>& outRe, QVector<double>& outIm)
{
    int N = inRe.size();
    // Conjugate input, forward transform, conjugate and scale
    QVector<double> conjIm(N);
    for (int i = 0; i < N; ++i) conjIm[i] = -inIm[i];
    transform(inRe, conjIm, outRe, outIm);
    for (int i = 0; i < N; ++i) {
        outIm[i] = -outIm[i];
        outRe[i] /= N;
        outIm[i] /= N;
    }
}

/* ---- Reset ---- */

void MixedRadixFFT4::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
