/**
 * @file BruunFFT5.cpp
 * @brief BruunFFT5 实现
 *
 * 实现Bruun实数FFT：z^N+1多项式因子分解、cos/sin分量提取、实数蝶形网络。
 */

#include "utils/fft204/BruunFFT5.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

BruunFFT5::BruunFFT5(QObject *parent) : QObject(parent) { precompute(m_size); }
BruunFFT5::~BruunFFT5() = default;

/* ---- Configuration ---- */

void BruunFFT5::setTransformSize(int n)
{
    // Round up to nearest power of 2
    int p = 1;
    while (p < n) p <<= 1;
    m_size = qMax(2, p);
    precompute(m_size);
}

/* ---- Precompute twiddle factors ---- */

void BruunFFT5::precompute(int n)
{
    m_cosTwiddle.resize(n);
    m_sinTwiddle.resize(n);
    for (int k = 0; k < n; ++k) {
        double angle = 2.0 * M_PI * k / (2 * n); // z^(2N) roots
        m_cosTwiddle[k] = qCos(angle);
        m_sinTwiddle[k] = qSin(angle);
    }
}

/* ---- Quadratic factors of z^N+1 ---- */

QVector<QPair<double, double>> BruunFFT5::quadraticFactors(int n) const
{
    // z^N + 1 = product of (z^2 - 2*cos((2k+1)*pi/N)*z + 1) for k=0..N/2-1
    QVector<QPair<double, double>> factors;
    factors.reserve(n / 2);
    for (int k = 0; k < n / 2; ++k) {
        double angle = (2 * k + 1) * M_PI / n;
        double coeff = -2.0 * qCos(angle);
        factors.append({coeff, 1.0}); // z^2 + coeff*z + 1
    }
    return factors;
}

/* ---- Butterfly stage ---- */

void BruunFFT5::butterflyStage(QVector<double>& data, int stage) const
{
    int blockSize = 1 << stage;
    int numBlocks = data.size() / (2 * blockSize);

    for (int b = 0; b < numBlocks; ++b) {
        int base = b * 2 * blockSize;
        for (int i = 0; i < blockSize; ++i) {
            int idx1 = base + i;
            int idx2 = base + blockSize + i;
            int twIdx = i * (data.size() / (2 * blockSize));

            double a = data[idx1];
            double c = data[idx2];
            double tw = m_cosTwiddle[twIdx % m_cosTwiddle.size()];

            // Bruun butterfly: real-only operations
            data[idx1] = a + c;
            data[idx2] = tw * (a - c);
        }
    }
}

/* ---- Extract cos/sin components ---- */

void BruunFFT5::extractCosSin(const QVector<double>& butterflies,
                                 QVector<double>& cosOut, QVector<double>& sinOut) const
{
    int n = butterflies.size();
    int half = n / 2;
    cosOut.resize(half);
    sinOut.resize(half);

    // DC component
    cosOut[0] = butterflies[0] / n;

    for (int k = 1; k < half; ++k) {
        double re = butterflies[2 * k - 1];
        double im = butterflies[2 * k];
        cosOut[k] = re / n;
        sinOut[k] = -im / n;
    }

    // Nyquist
    cosOut[0] = butterflies[0] / n;
}

/* ---- Forward transform ---- */

QVector<double> BruunFFT5::forward(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    int n = input.size();
    if (n != m_size) { m_size = n; precompute(n); }

    // Bruun's algorithm: evaluate X(z) at roots of z^N+1
    // using polynomial factorization mod (z^N+1)
    QVector<double> data = input;

    // Log2(N) stages of Bruun butterflies
    int log2N = 0;
    int temp = n;
    while (temp > 1) { log2N++; temp >>= 1; }

    for (int stage = 0; stage < log2N; ++stage)
        butterflyStage(data, stage);

    // Post-process: separate cos/sin spectral components
    // Using z^N+1 factorization, even outputs are cos, odd are sin
    QVector<double> result(n);
    for (int k = 0; k < n / 2; ++k) {
        int twIdx = k;
        double c = m_cosTwiddle[twIdx % m_cosTwiddle.size()];
        double s = m_sinTwiddle[twIdx % m_sinTwiddle.size()];

        // Symmetric combination
        double even = (k < data.size()) ? data[k] : 0.0;
        double odd = (k + n / 2 < data.size()) ? data[k + n / 2] : 0.0;

        result[2 * k] = even * c + odd * s;        // real (cos) part
        result[2 * k + 1] = -even * s + odd * c;    // imag (sin) part
    }

    m_stats.totalTransforms++;
    m_stats.transformSize = n;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalTransforms;

    emit transformCompleted(n, timer.elapsed());
    return result;
}

/* ---- Inverse transform ---- */

QVector<double> BruunFFT5::inverse(const QVector<double>& cosCoeff, const QVector<double>& sinCoeff)
{
    QElapsedTimer timer;
    timer.start();

    int half = cosCoeff.size();
    int n = 2 * half;
    if (n != m_size) { m_size = n; precompute(n); }

    // Reconstruct complex spectrum and invert
    QVector<double> data(n, 0.0);

    // Reverse the Bruun decomposition
    for (int k = 0; k < half; ++k) {
        int twIdx = k;
        double c = m_cosTwiddle[twIdx % m_cosTwiddle.size()];
        double s = m_sinTwiddle[twIdx % m_sinTwiddle.size()];

        double re = cosCoeff[k] * n;
        double im = sinCoeff[k] * n;

        data[k] = re * c - im * s;
        if (k + half < n)
            data[k + half] = re * s + im * c;
    }

    // Inverse butterfly stages (reverse order)
    int log2N = 0;
    int temp = n;
    while (temp > 1) { log2N++; temp >>= 1; }

    for (int stage = log2N - 1; stage >= 0; --stage) {
        int blockSize = 1 << stage;
        int numBlocks = n / (2 * blockSize);

        for (int b = 0; b < numBlocks; ++b) {
            int base = b * 2 * blockSize;
            for (int i = 0; i < blockSize; ++i) {
                int idx1 = base + i;
                int idx2 = base + blockSize + i;
                int twIdx = i * (n / (2 * blockSize));
                double tw = m_cosTwiddle[twIdx % m_cosTwiddle.size()];

                double sum = data[idx1];
                double diff = data[idx2] / qMax(tw, 1e-15);
                data[idx1] = (sum + diff) * 0.5;
                data[idx2] = (sum - diff) * 0.5;
            }
        }
    }

    m_stats.totalTransforms++;
    m_stats.transformSize = n;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalTransforms;

    emit transformCompleted(n, timer.elapsed());
    return data;
}

/* ---- Reset ---- */

void BruunFFT5::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
