/**
 * @file Resampler10.cpp
 * @brief Resampler10 实现
 *
 * 实现重采样器：Farrow结构与Lagrange多项式插值的连续时间分数延迟。
 */

#include "utils/signal282/Resampler10.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

Resampler10::Resampler10(QObject *parent)
    : QObject(parent)
{
    computeFarrowCoeffs();
}

Resampler10::~Resampler10() = default;

/* ---- Configuration ---- */

void Resampler10::setFilterOrder(int order)
{
    m_farrow.order = qBound(1, order, 7);
    computeFarrowCoeffs();
}

/* ---- Compute Lagrange-Farrow coefficients ---- */

void Resampler10::computeFarrowCoeffs()
{
    int n = m_farrow.order + 1;
    m_farrow.c = QVector<QVector<double>>(n, QVector<double>(n, 0.0));

    // Farrow structure: compute polynomial coefficients from Lagrange basis
    // y(mu) = sum_k c[k][j] * mu^j for each tap k
    // c[k][j] = sum over basis polynomials
    for (int k = 0; k < n; ++k) {
        // Lagrange basis L_k(mu) = product_{j!=k} (mu - j)/(k - j)
        // Expand into polynomial coefficients via Horner's method
        QVector<double> poly(n, 0.0);
        poly[0] = 1.0;  // Start with constant 1

        for (int j = 0; j < n; ++j) {
            if (j == k) continue;
            double denom = static_cast<double>(k - j);
            // Multiply poly by (mu - j) / denom
            QVector<double> newPoly(n, 0.0);
            for (int p = n - 1; p >= 0; --p) {
                newPoly[p + 1 < n ? p + 1 : 0] += poly[p] / denom;
                newPoly[p] += poly[p] * (-j) / denom;
            }
            // Shift: multiply by mu
            QVector<double> shifted(n, 0.0);
            shifted[0] = 0.0;
            for (int p = 0; p < n - 1; ++p)
                shifted[p + 1] = newPoly[p];
            // Add constant term
            shifted[0] += newPoly[0];
            // Actually redo properly
            poly = QVector<double>(n, 0.0);
            for (int p = 0; p < n; ++p) {
                poly[p] += newPoly[p];
                if (p > 0) poly[p] += newPoly[p - 1];
            }
            // Simplified: just accumulate the direct coefficient
            poly.fill(0.0);
            for (int p = 0; p < n; ++p) {
                if (p > 0) poly[p] += shifted[p];
                poly[0] += newPoly[0] * (-j) / denom;
            }
        }

        // Store: c[k] = polynomial coefficients of L_k
        for (int j = 0; j < n; ++j)
            m_farrow.c[k][j] = poly[j];
    }

    // Use simpler direct computation for correctness
    // Recompute with straightforward Lagrange coefficient extraction
    for (int j = 0; j < n; ++j) {
        for (int k = 0; k < n; ++k) {
            // Compute coefficient of mu^j in Lagrange basis L_k(mu)
            // Using finite differences on the sampling points
            double coeff = 0.0;
            // Evaluate j-th derivative at mu=0 scaled by 1/j!
            QVector<double> vals(n, 0.0);
            vals[k] = 1.0;
            // Repeated finite differences give polynomial coefficients
            for (int iter = 0; iter < j; ++iter) {
                for (int p = n - 1; p > 0; --p)
                    vals[p] = vals[p] - vals[p - 1];
            }
            double fact = 1.0;
            for (int f = 2; f <= j; ++f) fact *= f;
            coeff = (j > 0) ? vals[j] / fact : vals[0];
            m_farrow.c[k][j] = coeff;
        }
    }
}

/* ---- Lagrange basis value at mu ---- */

double Resampler10::lagrangeBasis(int k, int n, double mu) const
{
    double result = 1.0;
    for (int j = 0; j < n; ++j) {
        if (j == k) continue;
        double denom = static_cast<double>(k - j);
        if (qAbs(denom) < 1e-15) continue;
        result *= (mu - j) / denom;
    }
    return result;
}

/* ---- Get samples around position ---- */

QVector<double> Resampler10::getSamples(const QVector<double>& input,
                                          int centerIdx, int halfLen) const
{
    int n = input.size();
    int totalLen = 2 * halfLen + 1;
    QVector<double> samples(totalLen, 0.0);
    for (int i = 0; i < totalLen; ++i) {
        int idx = centerIdx - halfLen + i;
        if (idx >= 0 && idx < n) samples[i] = input[idx];
    }
    return samples;
}

/* ---- Farrow output for single fractional position ---- */

double Resampler10::farrowOutput(const QVector<double>& samples, double mu) const
{
    int n = qMin(samples.size(), m_farrow.order + 1);
    double result = 0.0;

    // Direct Lagrange interpolation
    for (int k = 0; k < n; ++k)
        result += samples[k] * lagrangeBasis(k, n, mu);

    return result;
}

/* ---- Resample by arbitrary ratio ---- */

Resampler10::ResampleResult Resampler10::resample(const QVector<double>& input, double ratio)
{
    QElapsedTimer timer;
    timer.start();

    ResampleResult result;
    int inN = input.size();
    if (inN == 0 || ratio <= 0.0) return result;

    int outN = qMax(1, static_cast<int>(qRound(inN * ratio)));
    result.output.resize(outN, 0.0);
    result.inputSize = inN;
    result.outputSize = outN;
    result.ratio = ratio;

    int halfOrder = m_farrow.order / 2;

    for (int i = 0; i < outN; ++i) {
        // Map output index to input position
        double inPos = static_cast<double>(i) / ratio;
        int intPos = static_cast<int>(inPos);
        double frac = inPos - intPos;

        // Clamp to valid range
        if (intPos >= inN) { result.output[i] = 0.0; continue; }

        // Get surrounding samples
        QVector<double> samples = getSamples(input, intPos + halfOrder, halfOrder);

        // Interpolate using Farrow/Lagrange
        result.output[i] = farrowOutput(samples, frac);
    }

    double elapsed = timer.elapsed();
    m_stats.inputSamples = inN;
    m_stats.outputSamples = outN;
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit resampleDone(inN, outN, ratio, elapsed);

    return result;
}

/* ---- Apply fractional delay ---- */

QVector<double> Resampler10::fractionalDelay(const QVector<double>& input, double delay) const
{
    int n = input.size();
    QVector<double> output(n, 0.0);
    int halfOrder = m_farrow.order / 2;

    for (int i = 0; i < n; ++i) {
        double pos = static_cast<double>(i) - delay;
        int intPos = static_cast<int>(qFloor(pos));
        double frac = pos - intPos;

        if (intPos < 0 || intPos >= n) { output[i] = 0.0; continue; }

        QVector<double> samples = getSamples(input, intPos + halfOrder, halfOrder);
        output[i] = farrowOutput(samples, frac);
    }
    return output;
}

/* ---- Reset ---- */

void Resampler10::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
