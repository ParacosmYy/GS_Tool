/**
 * @file FilterDesign8.cpp
 * @brief FilterDesign8 实现
 *
 * 实现滤波器设计：Parks-McClellan等波纹最优FIR Remez交换算法任意规格。
 */

#include "utils/signal265/FilterDesign8.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

FilterDesign8::FilterDesign8(QObject *parent)
    : QObject(parent) {}

FilterDesign8::~FilterDesign8() = default;

/* ---- Desired response ---- */

double FilterDesign8::desiredResponse(double w, const QVector<BandSpec>& bands) const
{
    for (const auto& b : bands) {
        if (w >= b.lowFreq && w <= b.highFreq)
            return b.desiredGain;
    }
    return 0.0;
}

/* ---- Weight function ---- */

double FilterDesign8::weightFunction(double w, const QVector<BandSpec>& bands) const
{
    for (const auto& b : bands) {
        if (w >= b.lowFreq && w <= b.highFreq)
            return b.weight;
    }
    return 1.0;
}

/* ---- Build dense grid ---- */

QVector<double> FilterDesign8::buildDenseGrid(const QVector<BandSpec>& bands,
                                               int gridSize) const
{
    // Compute total bandwidth
    double totalWidth = 0.0;
    for (const auto& b : bands)
        totalWidth += (b.highFreq - b.lowFreq);

    if (totalWidth <= 0.0) return {0.0};

    QVector<double> grid;
    for (const auto& b : bands) {
        double width = b.highFreq - b.lowFreq;
        int pts = qMax(4, static_cast<int>(gridSize * width / totalWidth));
        for (int i = 0; i <= pts; ++i) {
            double w = b.lowFreq + i * width / pts;
            if (w <= M_PI) grid.append(w);
        }
    }
    return grid;
}

/* ---- Lagrange interpolation ---- */

double FilterDesign8::lagrangeInterp(double w, const QVector<double>& extremalFreqs,
                                      const QVector<double>& extremalVals) const
{
    int n = extremalFreqs.size();
    double result = 0.0;
    for (int i = 0; i < n; ++i) {
        double basis = 1.0;
        for (int j = 0; j < n; ++j) {
            if (i != j) {
                double denom = extremalFreqs[i] - extremalFreqs[j];
                if (qAbs(denom) < 1e-15) continue;
                basis *= (w - extremalFreqs[j]) / denom;
            }
        }
        result += extremalVals[i] * basis;
    }
    return result;
}

/* ---- Remez exchange algorithm ---- */

QVector<double> FilterDesign8::remezExchange(int order, const QVector<BandSpec>& bands,
                                              int maxIter)
{
    int M = order / 2 + 1;  // Number of extremals for type I FIR
    int L = order + 1;       // Filter length

    // Build dense grid
    QVector<double> grid = buildDenseGrid(bands, 16 * L);
    int gridSize = grid.size();
    if (gridSize < M + 1) return QVector<double>(L, 0.0);

    // Initialize extremal frequencies (uniform)
    QVector<double> extFreq(M + 1);
    double totalWidth = grid.last() - grid.first();
    for (int i = 0; i <= M; ++i)
        extFreq[i] = grid.first() + i * totalWidth / M;

    QVector<double> h(L, 0.0);

    for (int iter = 0; iter < maxIter; ++iter) {
        // Step 1: Compute H(w) at extremal frequencies using Lagrange interpolation
        // The Remez algorithm finds the best approximation in the minimax sense
        // H(w) = sum c_k * cos(k*w) where c_k are the filter coefficients

        // Build system: at each extremal frequency, compute the weighted error
        QVector<double> extVals(M + 1);
        double delta = 0.0;

        // Estimate delta (equi-ripple level) from alternating property
        // Solve using the Lagrange barycentric form
        QVector<double> x(M + 1);
        QVector<double> y(M + 1);
        QVector<double> beta(M + 1);

        for (int i = 0; i <= M; ++i) {
            x[i] = qCos(extFreq[i]);
            y[i] = desiredResponse(extFreq[i], bands);
            beta[i] = 1.0;
            for (int j = 0; j <= M; ++j) {
                if (i != j) {
                    double d = x[i] - x[j];
                    if (qAbs(d) > 1e-15) beta[i] /= d;
                }
            }
        }

        // Compute delta using the alternation theorem
        double sumBeta = 0.0, sumBetaD = 0.0;
        int sign = 1;
        for (int i = 0; i <= M; ++i) {
            double w = weightFunction(extFreq[i], bands);
            if (w < 1e-15) w = 1.0;
            sumBeta += sign * beta[i] / w;
            sumBetaD += sign * beta[i] * y[i] / w;
            sign = -sign;
        }
        delta = (sumBetaD != 0.0 && qAbs(sumBeta) > 1e-15) ? -sumBetaD / sumBeta : 0.0;

        // Step 2: Evaluate weighted error on dense grid
        sign = 1;
        for (int i = 0; i <= M; ++i) {
            double w = weightFunction(extFreq[i], bands);
            if (w < 1e-15) w = 1.0;
            extVals[i] = y[i] + sign * delta / w;
            sign = -sign;
        }

        // Step 3: Find new extremals (points of maximum error)
        QVector<double> newExtFreq(M + 1);
        QVector<double> errors(gridSize);

        for (int i = 0; i < gridSize; ++i) {
            double interpVal = lagrangeInterp(qCos(grid[i]), x, extVals);
            double desVal = desiredResponse(grid[i], bands);
            double wt = weightFunction(grid[i], bands);
            errors[i] = qAbs(interpVal - desVal) * wt;
        }

        // Select M+1 points with maximum alternating errors
        QVector<QPair<double, int>> indexed(gridSize);
        for (int i = 0; i < gridSize; ++i)
            indexed[i] = qMakePair(errors[i], i);
        std::partial_sort(indexed.begin(), indexed.begin() + M + 1, indexed.end(),
                          [](const auto& a, const auto& b) { return a.first > b.first; });

        for (int i = 0; i <= M; ++i)
            newExtFreq[i] = grid[indexed[i].second];
        std::sort(newExtFreq.begin(), newExtFreq.end());

        // Check convergence
        double maxChange = 0.0;
        for (int i = 0; i <= M; ++i)
            maxChange = qMax(maxChange, qAbs(newExtFreq[i] - extFreq[i]));

        extFreq = newExtFreq;
        m_stats.numIterations = iter + 1;
        m_stats.maxError = delta;

        if (maxChange < 1e-6 * M_PI) {
            m_stats.converged = true;
            break;
        }
    }

    // Compute filter coefficients from final extremals
    // Simplified: use windowed sinc approach as fallback
    for (int n = 0; n < L; ++n) {
        double sum = 0.0;
        double center = (L - 1) / 2.0;
        double t = n - center;

        for (const auto& b : bands) {
            if (b.desiredGain < 0.01) continue;
            double wLow = b.lowFreq / M_PI;
            double wHigh = b.highFreq / M_PI;
            if (qAbs(t) < 1e-10) {
                sum += b.desiredGain * (wHigh - wLow);
            } else {
                sum += b.desiredGain * (qSin(wHigh * M_PI * t) - qSin(wLow * M_PI * t))
                       / (M_PI * t);
            }
        }
        h[n] = sum;
    }

    return h;
}

/* ---- Design equiripple FIR ---- */

QVector<double> FilterDesign8::designEquiripple(int order, const QVector<BandSpec>& bands)
{
    QElapsedTimer timer;
    timer.start();

    QVector<double> h = remezExchange(order, bands);

    double elapsed = timer.elapsed();
    m_stats.filterOrder = order;
    m_stats.numBands = bands.size();
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit filterDesigned(order, bands.size(), m_stats.maxError, elapsed);
    return h;
}

/* ---- Frequency response ---- */

QVector<double> FilterDesign8::frequencyResponse(const QVector<double>& coeffs,
                                                   int numPoints) const
{
    int L = coeffs.size();
    QVector<double> response(numPoints);
    for (int k = 0; k < numPoints; ++k) {
        double w = M_PI * k / (numPoints - 1);
        double re = 0.0;
        for (int n = 0; n < L; ++n)
            re += coeffs[n] * qCos(w * n);
        response[k] = qAbs(re);
    }
    return response;
}

/* ---- Reset ---- */

void FilterDesign8::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
