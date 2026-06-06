/**
 * @file SplitRadixFFT.cpp
 * @brief SplitRadixFFT 实现
 *
 * 实现分裂基FFT：L型蝶形分解、递归N/2+N/4+N/4结构、位反转重排。
 */

#include "utils/fft173/SplitRadixFFT.h"

#include <QElapsedTimer>
#include <QtMath>

/* ---- Construction / Destruction ---- */

SplitRadixFFT::SplitRadixFFT(QObject *parent)
    : QObject(parent)
{
}

SplitRadixFFT::~SplitRadixFFT() = default;

/* ---- Utility ---- */

bool SplitRadixFFT::isPowerOfTwo(int n)
{
    return n > 0 && (n & (n - 1)) == 0;
}

int SplitRadixFFT::log2Int(int n)
{
    int log = 0;
    while ((1 << log) < n) ++log;
    return log;
}

/* ---- Bit-reverse permutation ---- */

void SplitRadixFFT::bitReverse(QVector<double>& data)
{
    int n = data.size() / 2;
    int bits = log2Int(n);

    for (int i = 0; i < n; ++i) {
        int rev = 0;
        for (int b = 0; b < bits; ++b) {
            if (i & (1 << b))
                rev |= (1 << (bits - 1 - b));
        }
        if (i < rev) {
            /* Swap real */
            qSwap(data[2 * i], data[2 * rev]);
            /* Swap imag */
            qSwap(data[2 * i + 1], data[2 * rev + 1]);
        }
    }
}

/* ---- Core split-radix recursive butterfly ---- */

void SplitRadixFFT::splitRadixCore(double* real, double* imag, int n, int stride)
{
    if (n <= 1) return;

    if (n == 2) {
        /* Simple 2-point DFT */
        double r0 = real[0], r1 = real[stride];
        double i0 = imag[0], i1 = imag[stride];
        real[0] = r0 + r1;
        imag[0] = i0 + i1;
        real[stride] = r0 - r1;
        imag[stride] = i0 - i1;
        return;
    }

    if (n == 4) {
        /* 4-point DFT (radix-4 butterfly) */
        double s = stride;
        double r0 = real[0], r1 = real[s], r2 = real[2 * s], r3 = real[3 * s];
        double i0 = imag[0], i1 = imag[s], i2 = imag[2 * s], i3 = imag[3 * s];

        /* Stage 1 */
        double t0r = r0 + r2, t0i = i0 + i2;
        double t1r = r0 - r2, t1i = i0 - i2;
        double t2r = r1 + r3, t2i = i1 + i3;
        double t3r = r1 - r3, t3i = i1 - i3;

        /* Stage 2 */
        real[0] = t0r + t2r;       imag[0] = t0i + t2i;
        real[s] = t1r + t3i;       imag[s] = t1i - t3r;
        real[2 * s] = t0r - t2r;   imag[2 * s] = t0i - t2i;
        real[3 * s] = t1r - t3i;   imag[3 * s] = t1i + t3r;
        return;
    }

    /* Split into one N/2 and two N/4 sub-transforms */
    int n2 = n / 2;
    int n4 = n / 4;

    /* Even part: N/2 DFT */
    splitRadixCore(real, imag, n2, stride * 2);

    /* Odd part: two N/4 DFTs at stride*2 + stride and stride*2 + 3*stride */
    /* But first, L-shaped butterfly on the odd part */
    double angle = -2.0 * M_PI / n;
    for (int k = 0; k < n4; ++k) {
        double w = angle * k;
        double cs = qCos(w);
        double sn = qSin(w);

        int idx1 = (2 * k + 1) * stride;
        int idx3 = (2 * k + 1 + n2) * stride;

        /* Twiddle for N/4 sub-transforms */
        double r1 = real[idx1], i1 = imag[idx1];
        double r3 = real[idx3], i3 = imag[idx3];

        /* Complex multiply by twiddle for k */
        double tr1 = r1 * cs - i1 * sn;
        double ti1 = r1 * sn + i1 * cs;

        /* Complex multiply by twiddle for 3k */
        double w3 = angle * 3 * k;
        double cs3 = qCos(w3);
        double sn3 = qSin(w3);
        double tr3 = r3 * cs3 - i3 * sn3;
        double ti3 = r3 * sn3 + i3 * cs3;

        /* L-shaped butterfly: combine into even-odd and odd-odd */
        real[idx1] = tr1 + tr3;
        imag[idx1] = ti1 + ti3;
        real[idx3] = tr1 - tr3;
        imag[idx3] = ti1 - ti3;
    }

    /* Recursive N/4 transforms */
    splitRadixCore(real + stride, imag + stride, n4, stride * 2);
    splitRadixCore(real + stride + n2 * stride, imag + stride + n2 * stride, n4, stride * 2);

    /* Combine results */
    for (int k = 0; k < n4; ++k) {
        int eIdx = 2 * k * stride;
        int o1Idx = (2 * k + 1) * stride;
        int o2Idx = (2 * k + 1 + n2) * stride;

        double re = real[eIdx];
        double ie = imag[eIdx];
        double ro1 = real[o1Idx];
        double io1 = imag[o1Idx];
        double ro2 = real[o2Idx];
        double io2 = imag[o2Idx];

        real[eIdx] = re + ro1 + ro2;
        imag[eIdx] = ie + io1 + io2;

        /* k-th output */
        double w = angle * k;
        double cs = qCos(w);
        double sn = qSin(w);

        real[o1Idx] = re + (ro1 * cs - io1 * sn) + (ro2 * cs - io2 * sn);
        imag[o1Idx] = ie + (ro1 * sn + io1 * cs) + (ro2 * sn + io2 * cs);

        double w3 = angle * 3 * k;
        double cs3 = qCos(w3);
        double sn3 = qSin(w3);

        real[o2Idx] = re + (ro1 * cs3 - io1 * sn3) + (ro2 * cs3 - io2 * sn3);
        imag[o2Idx] = ie + (ro1 * sn3 + io1 * cs3) + (ro2 * sn3 + io2 * cs3);
    }
}

/* ---- Forward transform ---- */

bool SplitRadixFFT::transform(QVector<double>& real, QVector<double>& imag)
{
    int n = real.size();
    if (!isPowerOfTwo(n) || n < 2) return false;
    if (imag.size() != n) return false;

    QElapsedTimer timer;
    timer.start();

    /* Interleave real and imag for bit-reverse */
    QVector<double> interleaved(2 * n);
    for (int i = 0; i < n; ++i) {
        interleaved[2 * i] = real[i];
        interleaved[2 * i + 1] = imag[i];
    }

    bitReverse(interleaved);

    /* Extract back */
    for (int i = 0; i < n; ++i) {
        real[i] = interleaved[2 * i];
        imag[i] = interleaved[2 * i + 1];
    }

    /* Iterative split-radix using L-shaped butterfly */
    for (int m = 2; m <= n; m *= 2) {
        int m4 = m / 4;
        int m2 = m / 2;

        for (int k = 0; k < n; k += m) {
            for (int j = 0; j < m4; ++j) {
                int j1 = k + j;
                int j2 = j1 + m4;
                int j3 = j2 + m4;
                int j4 = j3 + m4;

                double angle = -2.0 * M_PI * j / m;

                /* Twiddle for odd-1 */
                double cs1 = qCos(angle);
                double sn1 = qSin(angle);
                double x1r = real[j3] * cs1 - imag[j3] * sn1;
                double x1i = real[j3] * sn1 + imag[j3] * cs1;

                /* Twiddle for odd-3 */
                double angle3 = 3.0 * angle;
                double cs3 = qCos(angle3);
                double sn3 = qSin(angle3);
                double x3r = real[j4] * cs3 - imag[j4] * sn3;
                double x3i = real[j4] * sn3 + imag[j4] * cs3;

                /* L-shaped butterfly */
                double sum1r = x1r + x3r;
                double sum1i = x1i + x3i;
                double dif1r = x1r - x3r;
                double dif1i = x1i - x3i;

                real[j3] = real[j1] - sum1r;
                imag[j3] = imag[j1] - sum1i;
                real[j4] = real[j2] + dif1i;
                imag[j4] = imag[j2] - dif1r;

                real[j1] += sum1r;
                imag[j1] += sum1i;
                real[j2] -= dif1i;
                imag[j2] += dif1r;
            }
        }
    }

    m_stats.totalTransforms++;
    m_stats.lastN = n;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = (m_stats.totalTransforms > 0)
        ? m_timeSum / m_stats.totalTransforms : 0.0;

    emit transformCompleted(n);
    return true;
}

/* ---- Inverse transform ---- */

bool SplitRadixFFT::inverseTransform(QVector<double>& real, QVector<double>& imag)
{
    int n = real.size();
    /* Conjugate */
    for (int i = 0; i < n; ++i)
        imag[i] = -imag[i];

    if (!transform(real, imag)) return false;

    /* Scale and conjugate back */
    for (int i = 0; i < n; ++i) {
        real[i] /= n;
        imag[i] = -imag[i] / n;
    }
    return true;
}

/* ---- Statistics ---- */

void SplitRadixFFT::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
