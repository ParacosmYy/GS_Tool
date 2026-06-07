/**
 * @file FilterDesign2.cpp
 * @brief FilterDesign2 实现
 *
 * 实现滤波器设计：Parks-McClellan等波纹FIR、最小阶估计、频率响应分析。
 */

#include "utils/signal195/FilterDesign2.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

FilterDesign2::FilterDesign2(QObject *parent) : QObject(parent) {}
FilterDesign2::~FilterDesign2() = default;

/* ---- Configuration ---- */

void FilterDesign2::setFilterType(FilterType type) { m_type = type; }
void FilterDesign2::setSampleRate(double sr) { m_sampleRate = qMax(1.0, sr); }
void FilterDesign2::setRippleDb(double ripple) { m_rippleDb = qMax(0.001, ripple); }

/* ---- Estimate minimum filter order ---- */

int FilterDesign2::estimateOrder(double passbandFreq, double stopbandFreq,
                                   double passbandRipple, double stopbandAtten)
{
    // Bellanger's formula for minimum FIR order
    double deltaF = qAbs(stopbandFreq - passbandFreq) / m_sampleRate;
    if (deltaF < 1e-10) return 1024;

    double delta1 = qPow(10.0, -passbandRipple / 20.0);
    double delta2 = qPow(10.0, -stopbandAtten / 20.0);
    double deltaMin = qMin(delta1, delta2);

    double N = (-10.0 * qLog10(deltaMin * deltaMin) - 13.0) /
               (14.6 * deltaF);
    return qMax(10, static_cast<int>(qCeil(N)));
}

/* ---- Compute delta and Lagrange coefficients ---- */

double FilterDesign2::computeDelta(int r, const QVector<double>& xRef,
                                     const QVector<double>& dRef,
                                     const QVector<double>& wRef,
                                     QVector<double>& alpha) const
{
    // Solve system for delta and interpolant coefficients
    // Using simplified barycentric approach
    double num = 0.0, den = 0.0;
    for (int i = 0; i < r; ++i) {
        double prod = 1.0;
        for (int j = 0; j < r; ++j) {
            if (j != i)
                prod *= (xRef[i] - xRef[j]);
        }
        double gamma = 1.0 / prod;
        double sign = (i % 2 == 0) ? 1.0 : -1.0;
        num += gamma * dRef[i] * sign;
        den += gamma * sign / wRef[i];
    }
    double delta = num / den;

    // Compute alpha (Lagrange basis coefficients with Chebyshev alternation)
    alpha.resize(r);
    for (int i = 0; i < r; ++i) {
        double prod = 1.0;
        for (int j = 0; j < r; ++j) {
            if (j != i)
                prod *= (xRef[i] - xRef[j]);
        }
        alpha[i] = (dRef[i] - ((i % 2 == 0) ? delta : -delta) / wRef[i]) / prod;
    }
    return delta;
}

/* ---- Evaluate Lagrange interpolant ---- */

double FilterDesign2::evalInterpolant(double x, int r,
                                        const QVector<double>& xRef,
                                        const QVector<double>& alpha) const
{
    double num = 0.0, den = 0.0;
    for (int i = 0; i < r; ++i) {
        if (qAbs(x - xRef[i]) < 1e-15) {
            double val = 0.0;
            for (int k = 0; k < r; ++k)
                val += alpha[k];
            return val;  // simplified
        }
        double prod = 1.0;
        for (int j = 0; j < r; ++j) {
            if (j != i)
                prod *= (x - xRef[j]);
        }
        num += alpha[i] * prod;
    }
    return num;
}

/* ---- Parks-McClellan Remez exchange ---- */

QVector<double> FilterDesign2::remez(int order, int numBands,
                                        const QVector<double>& bandEdges,
                                        const QVector<double>& desired,
                                        const QVector<double>& weights)
{
    int r = order / 2 + 1;
    int gridSize = 0;
    for (int b = 0; b < numBands; ++b)
        gridSize += qMax(2, static_cast<int>(16 * r * (bandEdges[2 * b + 1] - bandEdges[2 * b])));

    gridSize = qBound(r + 1, gridSize, 4096);

    // Generate dense grid of frequencies
    QVector<double> gridFreq(gridSize);
    QVector<double> gridDesired(gridSize);
    QVector<double> gridWeight(gridSize);
    int gi = 0;

    for (int b = 0; b < numBands; ++b) {
        int pts = qMax(2, gridSize / numBands);
        for (int p = 0; p < pts && gi < gridSize; ++p) {
            double t = static_cast<double>(p) / (pts - 1);
            gridFreq[gi] = bandEdges[2 * b] + t * (bandEdges[2 * b + 1] - bandEdges[2 * b]);
            gridDesired[gi] = desired[b];
            gridWeight[gi] = weights[b];
            gi++;
        }
    }
    gridSize = gi;

    // Initial reference set: uniformly spaced
    QVector<double> xRef(r);
    for (int i = 0; i < r; ++i)
        xRef[i] = gridFreq[i * (gridSize - 1) / qMax(1, r - 1)];

    QVector<double> dRef(r), wRef(r);
    QVector<double> alpha;

    // Remez iteration
    for (int iter = 0; iter < 50; ++iter) {
        // Look up desired and weight at reference points
        for (int i = 0; i < r; ++i) {
            int idx = qBound(0, i * (gridSize - 1) / qMax(1, r - 1), gridSize - 1);
            dRef[i] = gridDesired[idx];
            wRef[i] = gridWeight[idx];
        }

        double delta = computeDelta(r, xRef, dRef, wRef, alpha);

        // Find extrema of error on dense grid
        QVector<QPair<double, double>> extremals;
        for (int gi2 = 0; gi2 < gridSize; ++gi2) {
            double H = evalInterpolant(gridFreq[gi2], r, xRef, alpha);
            double err = (H - gridDesired[gi2]) * gridWeight[gi2];
            extremals.append({gridFreq[gi2], err});
        }

        // Find peaks where error alternates sign
        QVector<int> peakIdx;
        for (int i = 1; i < extremals.size() - 1; ++i) {
            double prev = extremals[i - 1].second;
            double curr = extremals[i].second;
            double next = extremals[i + 1].second;
            if ((curr >= prev && curr >= next) || (curr <= prev && curr <= next))
                peakIdx.append(i);
        }

        if (peakIdx.size() < r) break;

        // Select r largest peaks with alternating sign
        QVector<int> bestPeaks;
        int sign = 1;
        for (int p = 0; p < peakIdx.size() && bestPeaks.size() < r; ++p) {
            int idx = peakIdx[p];
            double err = extremals[idx].second;
            if (bestPeaks.isEmpty() || (err > 0) == (sign > 0)) {
                bestPeaks.append(idx);
                sign = -sign;
            }
        }

        if (bestPeaks.size() < r) break;

        // Update reference set
        bool changed = false;
        for (int i = 0; i < r; ++i) {
            if (qAbs(xRef[i] - extremals[bestPeaks[i]].first) > 1e-10)
                changed = true;
            xRef[i] = extremals[bestPeaks[i]].first;
        }
        if (!changed) break;
    }

    // Compute filter coefficients from final interpolant
    QVector<double> h(order + 1, 0.0);
    for (int n = 0; n <= order; ++n) {
        double sum = 0.0;
        for (int k = 0; k < r; ++k) {
            double prod = 1.0;
            for (int j = 0; j < r; ++j) {
                if (j != k)
                    prod *= (static_cast<double>(n) - xRef[j]);
            }
            sum += alpha[k] * prod;
        }
        h[n] = sum;
    }

    // Normalize
    double maxVal = 0.0;
    for (double v : h) maxVal = qMax(maxVal, qAbs(v));
    if (maxVal > 1e-15)
        for (double& v : h) v /= maxVal;

    return h;
}

/* ---- Design ---- */

QVector<double> FilterDesign2::design(
    int order, const QVector<QPair<double, double>>& bands,
    const QVector<double>& desired, const QVector<double>& weights)
{
    QElapsedTimer timer;
    timer.start();

    int numBands = bands.size();
    if (numBands == 0 || desired.size() != numBands) return {};

    // Flatten band edges
    QVector<double> bandEdges(numBands * 2);
    for (int i = 0; i < numBands; ++i) {
        bandEdges[2 * i] = bands[i].first;
        bandEdges[2 * i + 1] = bands[i].second;
    }

    QVector<double> w = weights;
    if (w.size() != numBands)
        w = QVector<double>(numBands, 1.0);

    m_coeffs = remez(order, numBands, bandEdges, desired, w);

    m_stats.totalDesigns++;
    m_stats.filterOrder = order;
    m_stats.rippleDb = m_rippleDb;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalDesigns;

    emit designCompleted(order, m_rippleDb, timer.elapsed());
    return m_coeffs;
}

/* ---- Frequency response ---- */

QVector<QVector<double>> FilterDesign2::frequencyResponse(
    const QVector<double>& coeffs, int numPoints) const
{
    int N = coeffs.size();
    QVector<QVector<double>> response(numPoints, {0.0, 0.0});

    for (int k = 0; k < numPoints; ++k) {
        double freq = M_PI * k / (numPoints - 1);
        double re = 0.0, im = 0.0;
        for (int n = 0; n < N; ++n) {
            double angle = -freq * n;
            re += coeffs[n] * qCos(angle);
            im += coeffs[n] * qSin(angle);
        }
        response[k] = {re, im};
    }
    return response;
}

/* ---- Accessors ---- */

QVector<double> FilterDesign2::coefficients() const { return m_coeffs; }

/* ---- Reset ---- */

void FilterDesign2::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_coeffs.clear();
}
