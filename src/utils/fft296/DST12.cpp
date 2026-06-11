/**
 * @file DST12.cpp
 * @brief DST12 实现
 *
 * 实现离散正弦变换：FFT反射与奇对称扩展实现II型离散正弦变换。
 */

#include "utils/fft296/DST12.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

DST12::DST12(QObject *parent)
    : QObject(parent)
{
    precompute();
}

DST12::~DST12() = default;

/* ---- Configuration ---- */

void DST12::setSize(int N)
{
    int p = 1;
    while (p < N) p *= 2;
    m_N = qBound(4, p, 1 << 20);
    precompute();
}

/* ---- Precompute twiddle factors ---- */

void DST12::precompute()
{
    m_sinTwiddles.resize(m_N);
    for (int k = 0; k < m_N; ++k)
        m_sinTwiddles[k] = qSin(M_PI * (k + 1) / (m_N + 1));

    m_fftReal.resize(2 * m_N);
    m_fftImag.resize(2 * m_N);
}

/* ---- Bit reversal ---- */

int DST12::reverseBits(int val, int bits) const
{
    int result = 0;
    for (int i = 0; i < bits; ++i) {
        result = (result << 1) | (val & 1);
        val >>= 1;
    }
    return result;
}

void DST12::bitReversePermute(QVector<double>& real, QVector<double>& imag) const
{
    int N = real.size();
    int bits = 0;
    { int n = N; while (n > 1) { n >>= 1; bits++; } }

    for (int i = 0; i < N; ++i) {
        int j = reverseBits(i, bits);
        if (j > i) {
            std::swap(real[i], real[j]);
            std::swap(imag[i], imag[j]);
        }
    }
}

/* ---- In-place Cooley-Tukey radix-2 DIT FFT ---- */

void DST12::fftInPlace(QVector<double>& real, QVector<double>& imag) const
{
    int N = real.size();
    bitReversePermute(real, imag);

    for (int size = 2; size <= N; size *= 2) {
        int halfSize = size / 2;
        double angleStep = -2.0 * M_PI / size;
        for (int i = 0; i < N; i += size) {
            for (int j = 0; j < halfSize; ++j) {
                double angle = angleStep * j;
                double wr = qCos(angle);
                double wi = qSin(angle);

                double tR = real[i + j + halfSize] * wr - imag[i + j + halfSize] * wi;
                double tI = real[i + j + halfSize] * wi + imag[i + j + halfSize] * wr;

                real[i + j + halfSize] = real[i + j] - tR;
                imag[i + j + halfSize] = imag[i + j] - tI;
                real[i + j] += tR;
                imag[i + j] += tI;
            }
        }
    }
}

/* ---- Forward DST-II via FFT reflection ---- */

DST12::DSTResult DST12::forward(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    DSTResult result;

    // DST-II via FFT reflection and odd-symmetry extension:
    // Construct sequence y[k] of length 2N:
    //   y[k] = x[k]       for k=0..N-1
    //   y[2N-1-k] = -x[k] for k=0..N-1  (odd symmetry)
    // Then DST[k] = Im{FFT(y)}[k+1] * scaling

    int N2 = 2 * m_N;
    QVector<double> real(N2, 0.0);
    QVector<double> imag(N2, 0.0);

    for (int k = 0; k < m_N; ++k) {
        double val = (k < input.size()) ? input[k] : 0.0;
        real[k] = val;
        real[2 * m_N - 1 - k] = -val;
    }

    fftInPlace(real, imag);

    // Extract DST coefficients from imaginary part
    result.coefficients.resize(m_N);
    double peak = 0.0;
    double firstEnergy = 0.0;
    double totalEnergy = 0.0;

    // DST-II scaling factor
    double scale = qSqrt(2.0 / (m_N + 1.0));

    for (int k = 0; k < m_N; ++k) {
        double val = imag[k + 1] * scale;
        result.coefficients[k] = val;
        peak = qMax(peak, qAbs(val));

        double e = val * val;
        totalEnergy += e;
        if (k == 0) firstEnergy = e;
    }

    result.peakValue = peak;
    result.energyRatio = totalEnergy > 0 ? firstEnergy / totalEnergy : 0.0;

    double elapsed = timer.elapsed();
    m_stats.transformSize = m_N;
    m_stats.totalTransforms++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalTransforms;

    emit transformDone(m_N, peak, elapsed);
    return result;
}

/* ---- Inverse DST-II (DST-III) ---- */

QVector<double> DST12::inverse(const QVector<double>& coefficients)
{
    QElapsedTimer timer;
    timer.start();

    // DST-III is the inverse of DST-II (up to normalization)
    // x[n] = sum_k C[k] * sin(pi*(k+1)*(2n+1)/(2N+2)) * scale
    // Using the same FFT reflection in reverse

    int N2 = 2 * m_N;
    QVector<double> real(N2, 0.0);
    QVector<double> imag(N2, 0.0);

    double scale = qSqrt(2.0 / (m_N + 1.0));

    // Build imaginary part from coefficients
    for (int k = 0; k < m_N; ++k) {
        double c = (k < coefficients.size()) ? coefficients[k] / scale : 0.0;
        imag[k + 1] = c;
        imag[N2 - 1 - k] = -c;  // Odd symmetry
    }

    // Inverse FFT: conjugate, FFT, conjugate, scale
    for (int i = 0; i < N2; ++i)
        imag[i] = -imag[i];

    fftInPlace(real, imag);

    // Extract output from the real part (which should be 0 for pure DST)
    // and the imaginary part
    QVector<double> output(m_N, 0.0);
    for (int n = 0; n < m_N; ++n) {
        output[n] = -imag[n] / N2 / scale;
    }

    double elapsed = timer.elapsed();
    m_stats.totalTransforms++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalTransforms;

    emit transformDone(m_N, 0.0, elapsed);
    return output;
}

/* ---- Reset ---- */

void DST12::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
