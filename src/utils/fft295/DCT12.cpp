/**
 * @file DCT12.cpp
 * @brief DCT12 实现
 *
 * 实现离散余弦变换：FFT约化快速余弦变换与偶扩展对称性实现II型DCT。
 */

#include "utils/fft295/DCT12.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

DCT12::DCT12(QObject *parent)
    : QObject(parent)
{
    precompute();
}

DCT12::~DCT12() = default;

/* ---- Configuration ---- */

void DCT12::setSize(int N)
{
    // Round to power of 2
    int p = 1;
    while (p < N) p *= 2;
    m_N = qBound(4, p, 1 << 20);
    precompute();
}

/* ---- Precompute twiddle factors ---- */

void DCT12::precompute()
{
    m_cosTwiddles.resize(m_N);
    m_scaleFactors.resize(m_N);

    // DCT-II twiddle: cos(pi*k*(2n+1)/(2N)) via FFT reduction
    // The FFT reduction reorders the input with even-extension symmetry
    for (int k = 0; k < m_N; ++k) {
        m_cosTwiddles[k] = qCos(M_PI * k / (2.0 * m_N));
    }

    // Scaling: DCT-II uses 2/N for k>0 and 1/N for k=0 (orthonormal variant)
    m_scaleFactors[0] = 1.0 / qSqrt(static_cast<double>(m_N));
    for (int k = 1; k < m_N; ++k)
        m_scaleFactors[k] = qSqrt(2.0 / m_N);

    // Allocate FFT working buffers
    m_fftReal.resize(m_N);
    m_fftImag.resize(m_N);
}

/* ---- Bit reversal ---- */

int DCT12::reverseBits(int val, int bits) const
{
    int result = 0;
    for (int i = 0; i < bits; ++i) {
        result = (result << 1) | (val & 1);
        val >>= 1;
    }
    return result;
}

void DCT12::bitReversePermute(QVector<double>& real, QVector<double>& imag) const
{
    int bits = 0;
    { int n = m_N; while (n > 1) { n >>= 1; bits++; } }

    for (int i = 0; i < m_N; ++i) {
        int j = reverseBits(i, bits);
        if (j > i) {
            std::swap(real[i], real[j]);
            std::swap(imag[i], imag[j]);
        }
    }
}

/* ---- In-place Cooley-Tukey radix-2 DIT FFT ---- */

void DCT12::fftInPlace(QVector<double>& real, QVector<double>& imag) const
{
    bitReversePermute(real, imag);

    for (int size = 2; size <= m_N; size *= 2) {
        int halfSize = size / 2;
        double angleStep = -2.0 * M_PI / size;
        for (int i = 0; i < m_N; i += size) {
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

/* ---- Forward DCT-II via FFT reduction ---- */

DCT12::DCTResult DCT12::forward(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    DCTResult result;

    // Reorder input using even-extension symmetry:
    // y[k] = x[2k] for k=0..N/2-1, y[N-1-k] = x[2k+1] for k=0..N/2-1
    m_fftReal.resize(m_N);
    m_fftImag.resize(m_N);
    m_fftImag.fill(0.0);

    for (int k = 0; k < m_N / 2; ++k) {
        int idx = qMin(2 * k, qMax(0, input.size() - 1));
        m_fftReal[k] = (2 * k < input.size()) ? input[2 * k] : 0.0;
    }
    for (int k = 0; k < m_N / 2; ++k) {
        int srcIdx = qMin(2 * k + 1, qMax(0, input.size() - 1));
        m_fftReal[m_N - 1 - k] = (2 * k + 1 < input.size()) ? input[2 * k + 1] : 0.0;
    }

    // Run FFT on the reordered sequence
    fftInPlace(m_fftReal, m_fftImag);

    // Post-process: multiply by twiddle factors and extract real part
    // DCT[k] = Re{ FFT[k] * exp(-j*pi*k/(2N)) } * scale
    result.coefficients.resize(m_N);
    double peak = 0.0;
    double dcEnergy = 0.0;
    double totalEnergy = 0.0;

    for (int k = 0; k < m_N; ++k) {
        // Complex multiplication with twiddle
        double wr = m_cosTwiddles[k];
        double wi = -qSin(M_PI * k / (2.0 * m_N)); // conjugate
        double dctVal = m_fftReal[k] * wr - m_fftImag[k] * wi;
        // Apply scaling
        dctVal *= m_scaleFactors[k] * qSqrt(2.0);
        result.coefficients[k] = dctVal;
        peak = qMax(peak, qAbs(dctVal));

        double e = dctVal * dctVal;
        totalEnergy += e;
        if (k == 0) dcEnergy = e;
    }

    result.peakValue = peak;
    result.energyRatio = totalEnergy > 0 ? dcEnergy / totalEnergy : 0.0;

    double elapsed = timer.elapsed();
    m_stats.transformSize = m_N;
    m_stats.totalTransforms++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalTransforms;

    emit transformDone(m_N, peak, elapsed);
    return result;
}

/* ---- Inverse DCT-II (DCT-III) ---- */

QVector<double> DCT12::inverse(const QVector<double>& coefficients)
{
    QElapsedTimer timer;
    timer.start();

    // DCT-III: x[n] = sum_k C[k]*cos(pi*k*(2n+1)/(2N))
    // Using FFT reduction in reverse
    m_fftReal.resize(m_N);
    m_fftImag.resize(m_N);

    // Pre-multiply by twiddle factors
    for (int k = 0; k < m_N; ++k) {
        double c = (k < coefficients.size()) ? coefficients[k] : 0.0;
        // Undo scaling
        c /= m_scaleFactors[k] * qSqrt(2.0);
        // Multiply by conjugate twiddle
        double angle = M_PI * k / (2.0 * m_N);
        m_fftReal[k] = c * qCos(angle);
        m_fftImag[k] = c * qSin(angle);
    }

    // Inverse FFT
    for (int k = 0; k < m_N; ++k)
        m_fftImag[k] = -m_fftImag[k]; // Conjugate

    fftInPlace(m_fftReal, m_fftImag);

    // Scale by 1/N and conjugate back
    for (int k = 0; k < m_N; ++k) {
        m_fftReal[k] /= m_N;
        m_fftImag[k] = -m_fftImag[k] / m_N;
    }

    // Undo even-extension reordering
    QVector<double> output(m_N, 0.0);
    for (int k = 0; k < m_N / 2; ++k)
        output[2 * k] = m_fftReal[k];
    for (int k = 0; k < m_N / 2; ++k)
        output[2 * k + 1] = m_fftReal[m_N - 1 - k];

    double elapsed = timer.elapsed();
    m_stats.totalTransforms++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalTransforms;

    emit transformDone(m_N, 0.0, elapsed);
    return output;
}

/* ---- DC component ---- */

double DCT12::dcComponent(const QVector<double>& input) const
{
    double sum = 0.0;
    int n = qMin(input.size(), m_N);
    for (int i = 0; i < n; ++i)
        sum += input[i];
    return sum / m_N;
}

/* ---- Reset ---- */

void DCT12::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
