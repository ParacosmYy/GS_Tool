/**
 * @file BruunFFT8.cpp
 * @brief BruunFFT8 实现
 *
 * 实现Bruun FFT：多项式因式分解与递归cos/sin调制实值2幂变换。
 */

#include "utils/fft238/BruunFFT8.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

BruunFFT8::BruunFFT8(QObject *parent) : QObject(parent) {}
BruunFFT8::~BruunFFT8() = default;

/* ---- Configure ---- */

bool BruunFFT8::configure(int N)
{
    if (N < 4) return false;
    // Check power of 2
    int tmp = N;
    m_log2N = 0;
    while (tmp > 1) {
        if (tmp & 1) return false;
        tmp >>= 1;
        m_log2N++;
    }
    m_N = N;
    m_stats.transformSize = N;
    computeTwiddles();
    computeBitReversal();
    return true;
}

/* ---- Precompute twiddle factors ---- */

void BruunFFT8::computeTwiddles()
{
    int halfN = m_N / 2;
    m_twiddleCos.resize(halfN);
    m_twiddleSin.resize(halfN);
    // Bruun's polynomial roots: cos(2*pi*k/N) and sin(2*pi*k/N)
    for (int k = 0; k < halfN; ++k) {
        double angle = 2.0 * M_PI * k / m_N;
        m_twiddleCos[k] = qCos(angle);
        m_twiddleSin[k] = qSin(angle);
    }
}

/* ---- Bit-reversal table ---- */

void BruunFFT8::computeBitReversal()
{
    m_bitRev.resize(m_N);
    for (int i = 0; i < m_N; ++i) {
        int rev = 0, val = i;
        for (int b = 0; b < m_log2N; ++b) {
            rev = (rev << 1) | (val & 1);
            val >>= 1;
        }
        m_bitRev[i] = rev;
    }
}

/* ---- Bit-reversal permutation ---- */

void BruunFFT8::bitReversePermute(QVector<double>& re, QVector<double>& im) const
{
    for (int i = 0; i < m_N; ++i) {
        int j = m_bitRev[i];
        if (j > i) {
            std::swap(re[i], re[j]);
            std::swap(im[i], im[j]);
        }
    }
}

/* ---- Recursive Bruun butterfly ---- */

void BruunFFT8::bruunButterfly(double* re, double* im, int stride, int m, int offset) const
{
    if (m <= 1) return;

    // Bruun factorization: z^m - 1 = (z^{m/2} - 1)(z^{m/2} + 1)
    // Apply modulation via polynomial root twiddles
    int halfM = m / 2;
    for (int k = 0; k < halfM; ++k) {
        int idx1 = offset + k * stride;
        int idx2 = offset + (k + halfM) * stride;

        double a = re[idx1], b = im[idx1];
        double c = re[idx2], d = im[idx2];

        // Apply cos/sin modulation from polynomial root
        int twIdx = k * (m_N / m);
        if (twIdx < m_twiddleCos.size()) {
            double cosT = m_twiddleCos[twIdx];
            double sinT = m_twiddleSin[twIdx];
            double modRe = c * cosT - d * sinT;
            double modIm = c * sinT + d * cosT;

            re[idx1] = a + modRe;
            im[idx1] = b + modIm;
            re[idx2] = a - modRe;
            im[idx2] = b - modIm;
        } else {
            re[idx1] = a + c;
            im[idx1] = b + d;
            re[idx2] = a - c;
            im[idx2] = b - d;
        }
    }
}

/* ---- Recursive Bruun transform ---- */

void BruunFFT8::bruunRecurse(double* re, double* im, int n, int stride) const
{
    if (n <= 2) {
        // Base case: 2-point DFT
        double a = re[0], b = im[0];
        double c = re[stride], d = im[stride];
        re[0] = a + c; im[0] = b + d;
        re[stride] = a - c; im[stride] = b - d;
        return;
    }

    int half = n / 2;

    // Apply butterfly: polynomial factorization step
    bruunButterfly(re, im, stride, n, 0);

    // Recurse on even and odd polynomials
    bruunRecurse(re, im, half, stride * 2);
    bruunRecurse(re + half * stride, im + half * stride, half, stride * 2);
}

/* ---- Forward complex FFT ---- */

void BruunFFT8::forwardComplex(QVector<double>& re, QVector<double>& im)
{
    if (m_N == 0) return;
    QElapsedTimer timer;
    timer.start();

    bitReversePermute(re, im);
    bruunRecurse(re.data(), im.data(), m_N, 1);

    m_stats.numForward++;
    m_stats.totalOps++;
    m_stats.recursionDepth = m_log2N;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit forwardCompleted(m_N, timer.elapsed());
}

/* ---- Inverse complex FFT ---- */

void BruunFFT8::inverseComplex(QVector<double>& re, QVector<double>& im)
{
    if (m_N == 0) return;
    QElapsedTimer timer;
    timer.start();

    // Conjugate
    for (int i = 0; i < m_N; ++i) im[i] = -im[i];
    // Forward transform
    bitReversePermute(re, im);
    bruunRecurse(re.data(), im.data(), m_N, 1);
    // Scale and conjugate
    for (int i = 0; i < m_N; ++i) {
        re[i] /= m_N;
        im[i] = -im[i] / m_N;
    }

    m_stats.numInverse++;
    emit inverseCompleted(m_N, timer.elapsed());
}

/* ---- Pack real spectrum ---- */

QVector<double> BruunFFT8::packRealSpectrum(const QVector<double>& re,
                                             const QVector<double>& im) const
{
    int half = m_N / 2;
    QVector<double> packed(half * 2);
    for (int k = 0; k <= half; ++k) {
        packed[k * 2] = re[k];
        packed[k * 2 + 1] = im[k];
    }
    return packed;
}

/* ---- Unpack real spectrum ---- */

void BruunFFT8::unpackRealSpectrum(const QVector<double>& packed,
                                    QVector<double>& re, QVector<double>& im) const
{
    int half = m_N / 2;
    re.resize(m_N); im.resize(m_N);
    for (int k = 0; k <= half; ++k) {
        re[k] = packed[k * 2];
        im[k] = packed[k * 2 + 1];
    }
    for (int k = 1; k < half; ++k) {
        re[m_N - k] = re[k];
        im[m_N - k] = -im[k];
    }
}

/* ---- Forward real-valued FFT ---- */

QVector<double> BruunFFT8::forward(const QVector<double>& input)
{
    if (input.size() != m_N) return {};

    QVector<double> re = input, im(m_N, 0.0);
    forwardComplex(re, im);
    return packRealSpectrum(re, im);
}

/* ---- Inverse real-valued FFT ---- */

QVector<double> BruunFFT8::inverse(const QVector<double>& spectrum)
{
    QVector<double> re, im;
    unpackRealSpectrum(spectrum, re, im);
    inverseComplex(re, im);
    return re;
}

/* ---- Reset ---- */

void BruunFFT8::resetStatistics()
{
    m_twiddleCos.clear(); m_twiddleSin.clear(); m_bitRev.clear();
    m_N = 0; m_log2N = 0;
    m_stats = Stats{}; m_timeSum = 0.0;
}
