/**
 * @file BruunFFT7.cpp
 * @brief BruunFFT7 实现
 *
 * 实现Bruun FFT：DFT因式分解为实/虚多项式残差对、级联蝶形。
 */

#include "utils/fft224/BruunFFT7.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

BruunFFT7::BruunFFT7(QObject *parent) : QObject(parent) {}
BruunFFT7::~BruunFFT7() = default;

/* ---- Prepare ---- */

bool BruunFFT7::prepare(int n)
{
    if (n < 2) return false;
    // Verify power of 2
    int test = n;
    while (test > 1) {
        if (test % 2 != 0) return false;
        test /= 2;
    }

    m_n = n;
    m_stages = 0;
    int tmp = n;
    while (tmp > 1) { tmp /= 2; m_stages++; }

    m_stats.transformSize = n;
    m_stats.numStages = m_stages;

    // Precompute polynomial residue coefficients for each stage
    m_realResidue.resize(m_stages);
    m_imagResidue.resize(m_stages);

    int totalButterflies = 0;
    for (int s = 0; s < m_stages; ++s) {
        int blockSize = 1 << (s + 1);
        int halfBlock = blockSize / 2;
        int numTwiddles = halfBlock;

        m_realResidue[s].resize(numTwiddles);
        m_imagResidue[s].resize(numTwiddles);

        for (int k = 0; k < numTwiddles; ++k) {
            // Polynomial residue: z^(2N) - 1 = (z^N - 1)(z^N + 1)
            // Decompose into real/imag residue pairs
            double angle = -2.0 * M_PI * k / blockSize;
            m_realResidue[s][k] = qCos(angle);
            m_imagResidue[s][k] = qSin(angle);
        }
        totalButterflies += n / 2;
    }

    m_stats.numButterflies = totalButterflies;
    return true;
}

/* ---- Compute residue coefficients ---- */

void BruunFFT7::computeResidueCoeffs(int stage, int blockSize,
                                        QVector<double>& realCoeff,
                                        QVector<double>& imagCoeff) const
{
    int halfBlock = blockSize / 2;
    realCoeff.resize(halfBlock);
    imagCoeff.resize(halfBlock);
    for (int k = 0; k < halfBlock; ++k) {
        double angle = -2.0 * M_PI * k / blockSize;
        realCoeff[k] = qCos(angle);
        imagCoeff[k] = qSin(angle);
    }
}

/* ---- Bruun butterfly ---- */

void BruunFFT7::bruunButterfly(double* reA, double* imA,
                                  double* reB, double* imB,
                                  double realCoeff, double imagCoeff) const
{
    // Complex multiply: twiddle * B
    double tRe = realCoeff * (*reB) - imagCoeff * (*imB);
    double tIm = realCoeff * (*imB) + imagCoeff * (*reB);

    // Butterfly: A' = A + twiddle*B, B' = A - twiddle*B
    double sumRe = *reA + tRe;
    double sumIm = *imA + tIm;
    double diffRe = *reA - tRe;
    double diffIm = *imA - tIm;

    *reA = sumRe;
    *imA = sumIm;
    *reB = diffRe;
    *imB = diffIm;
}

/* ---- Bit-reversal permutation ---- */

void BruunFFT7::bitReverse(QVector<double>& re, QVector<double>& im) const
{
    int n = m_n;
    for (int i = 1, j = 0; i < n; ++i) {
        int bit = n >> 1;
        while (j & bit) { j ^= bit; bit >>= 1; }
        j ^= bit;
        if (i < j) {
            std::swap(re[i], re[j]);
            std::swap(im[i], im[j]);
        }
    }
}

/* ---- Apply cascade butterfly ---- */

void BruunFFT7::applyCascadeButterfly(QVector<double>& re,
                                        QVector<double>& im) const
{
    for (int s = 0; s < m_stages; ++s) {
        int blockSize = 1 << (s + 1);
        int halfBlock = blockSize / 2;
        int numGroups = m_n / blockSize;

        for (int g = 0; g < numGroups; ++g) {
            int base = g * blockSize;
            for (int k = 0; k < halfBlock; ++k) {
                bruunButterfly(&re[base + k], &im[base + k],
                               &re[base + halfBlock + k], &im[base + halfBlock + k],
                               m_realResidue[s][k], m_imagResidue[s][k]);
            }
        }
    }
}

/* ---- Forward FFT ---- */

QVector<double> BruunFFT7::forward(const QVector<double>& input) const
{
    QElapsedTimer timer;
    timer.start();

    int n = m_n;
    QVector<double> re(n, 0.0), im(n, 0.0);

    // De-interleave input
    for (int i = 0; i < qMin(input.size() / 2, n); ++i) {
        re[i] = input[2 * i];
        im[i] = (2 * i + 1 < input.size()) ? input[2 * i + 1] : 0.0;
    }

    // Bruun cascade butterfly stages
    const_cast<BruunFFT7*>(this)->applyCascadeButterfly(re, im);

    // Bit-reversal to natural order
    const_cast<BruunFFT7*>(this)->bitReverse(re, im);

    // Re-interleave output
    QVector<double> out(2 * n);
    for (int i = 0; i < n; ++i) { out[2 * i] = re[i]; out[2 * i + 1] = im[i]; }

    const_cast<BruunFFT7*>(this)->m_stats.totalOps++;
    const_cast<BruunFFT7*>(this)->m_timeSum += timer.elapsed();
    const_cast<BruunFFT7*>(this)->m_stats.avgProcessingTimeMs =
        m_timeSum / m_stats.totalOps;
    emit const_cast<BruunFFT7*>(this)->transformCompleted(n, timer.elapsed());
    return out;
}

/* ---- Inverse FFT ---- */

QVector<double> BruunFFT7::inverse(const QVector<double>& input) const
{
    int n = m_n;
    // Conjugate input
    QVector<double> conj(2 * n);
    for (int i = 0; i < n; ++i) {
        conj[2 * i] = (2 * i < input.size()) ? input[2 * i] : 0.0;
        conj[2 * i + 1] = (2 * i + 1 < input.size()) ? -input[2 * i + 1] : 0.0;
    }
    QVector<double> result = forward(conj);
    for (int i = 0; i < result.size(); ++i) result[i] /= n;
    for (int i = 0; i < n && 2 * i + 1 < result.size(); ++i)
        result[2 * i + 1] = -result[2 * i + 1];
    return result;
}

/* ---- Reset ---- */

void BruunFFT7::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_realResidue.clear();
    m_imagResidue.clear();
    m_stages = 0;
    m_n = 0;
}
