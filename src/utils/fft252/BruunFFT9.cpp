/**
 * @file BruunFFT9.cpp
 * @brief BruunFFT9 实现
 *
 * 实现Bruun FFT：递归多项式分解与N=2^k实输出优化。
 */

#include "utils/fft252/BruunFFT9.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

BruunFFT9::BruunFFT9(QObject *parent) : QObject(parent) {}
BruunFFT9::~BruunFFT9() = default;

/* ---- Check power of 2 ---- */

bool BruunFFT9::isPowerOf2(int n) { return n > 0 && (n & (n - 1)) == 0; }

/* ---- Factorize z^N - 1 into quadratic factors ---- */

void BruunFFT9::factorizePolynomial(int n)
{
    m_factors.clear();
    m_twiddles.clear();

    // z^N - 1 = product of (z^2 - 2*cos(2*pi*k/N)*z + 1) for k = 1..N/2-1
    // times (z-1)(z+1)
    int halfN = n / 2;
    m_factors.reserve(halfN);
    m_twiddles.reserve(halfN);

    for (int k = 1; k < halfN; ++k) {
        QuadFactor f;
        double angle = 2.0 * M_PI * k / n;
        f.a = 1.0;
        f.b = -2.0 * qCos(angle);
        f.c = 1.0;
        m_factors.append(f);
        m_twiddles.append(qCos(angle));
    }

    m_stats.numFactorizations++;
}

/* ---- Bit-reversal permutation ---- */

void BruunFFT9::bitReverse(QVector<double>& data) const
{
    int n = data.size();
    int bits = 0;
    for (int tmp = n; tmp > 1; tmp >>= 1) bits++;

    for (int i = 0; i < n; ++i) {
        int rev = 0;
        for (int b = 0; b < bits; ++b)
            if (i & (1 << b)) rev |= (1 << (bits - 1 - b));
        if (i < rev) std::swap(data[i], data[rev]);
    }
}

/* ---- Recursive Bruun butterfly ---- */

void BruunFFT9::butterfly(QVector<double>& re, QVector<double>& im,
                            int start, int length, int stride) const
{
    if (length <= 1) return;

    int halfLen = length / 2;
    double angle = -2.0 * M_PI * start / m_size;
    double wRe = qCos(angle);
    double wIm = qSin(angle);

    for (int i = 0; i < halfLen; ++i) {
        int idx1 = start + i;
        int idx2 = start + i + halfLen;

        // Bruun's quadratic evaluation
        double aRe = re[idx1];
        double bRe = re[idx2];

        // Apply quadratic factor: x_even + x_odd * twiddle
        double tRe = bRe * wRe;
        re[idx1] = aRe + tRe;
        re[idx2] = aRe - tRe;

        if (!im.isEmpty()) {
            double aIm = im[idx1];
            double bIm = im[idx2];
            double tIm = bIm * wRe;
            im[idx1] = aIm + tIm;
            im[idx2] = aIm - tIm;
        }

        m_stats.polynomialOps++;
    }

    butterfly(re, im, start, halfLen, stride * 2);
    butterfly(re, im, start + halfLen, halfLen, stride * 2);
}

/* ---- Real-output optimized butterfly ---- */

void BruunFFT9::realButterfly(QVector<double>& re, QVector<double>& im,
                                int n) const
{
    // Pack real signal: re[i] = input[i], im[i] = 0
    for (int i = 0; i < n; ++i) im[i] = 0.0;

    // Standard decimation-in-time with Bruun's polynomial approach
    for (int len = 2; len <= n; len <<= 1) {
        double baseAngle = -2.0 * M_PI / len;
        for (int i = 0; i < n; i += len) {
            for (int j = 0; j < len / 2; ++j) {
                double angle = baseAngle * j;
                double wRe = qCos(angle);
                double wIm = qSin(angle);

                double tRe = re[i+j+len/2] * wRe - im[i+j+len/2] * wIm;
                double tIm = re[i+j+len/2] * wIm + im[i+j+len/2] * wRe;

                re[i+j+len/2] = re[i+j] - tRe;
                im[i+j+len/2] = im[i+j] - tIm;
                re[i+j] += tRe;
                im[i+j] += tIm;
            }
        }
    }
}

/* ---- Prepare ---- */

bool BruunFFT9::prepare(int n)
{
    if (!isPowerOf2(n) || n < 4) return false;
    m_size = n;
    factorizePolynomial(n);
    m_stats.transformSize = n;
    return true;
}

/* ---- Forward FFT ---- */

QVector<double> BruunFFT9::forward(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    int n = m_size;
    if (n <= 0) return {};

    QVector<double> re(n), im(n);
    for (int i = 0; i < qMin(input.size(), n); ++i)
        re[i] = input[i];

    // Bit-reverse
    bitReverse(re);

    // Apply real-output optimized Bruun butterfly
    im.resize(n);
    realButterfly(re, im, n);

    // Interleaved output
    QVector<double> output(n * 2);
    for (int i = 0; i < n; ++i) {
        output[i * 2] = re[i];
        output[i * 2 + 1] = im[i];
    }

    m_stats.numTransforms++;
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit transformCompleted(n, true, timer.elapsed());
    return output;
}

/* ---- Inverse FFT ---- */

QVector<double> BruunFFT9::inverse(const QVector<double>& spectrum)
{
    QElapsedTimer timer;
    timer.start();

    int n = m_size;
    if (n <= 0) return {};

    QVector<double> re(n), im(n);
    for (int i = 0; i < n; ++i) {
        re[i] = (i * 2 < spectrum.size()) ? spectrum[i * 2] : 0.0;
        im[i] = (i * 2 + 1 < spectrum.size()) ? spectrum[i * 2 + 1] : 0.0;
    }

    // Conjugate
    for (auto& v : im) v = -v;

    bitReverse(re);
    // Forward FFT on conjugated data
    QVector<double> imZero(n, 0.0);
    for (int i = 0; i < n; ++i) imZero[i] = im[i];
    realButterfly(re, imZero, n);

    QVector<double> output(n);
    for (int i = 0; i < n; ++i)
        output[i] = re[i] / n;

    m_stats.numTransforms++;
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit transformCompleted(n, false, timer.elapsed());
    return output;
}

/* ---- Reset ---- */

void BruunFFT9::resetStatistics()
{
    m_factors.clear();
    m_twiddles.clear();
    m_size = 0;
    m_stats = Stats{};
    m_timeSum = 0.0;
}
