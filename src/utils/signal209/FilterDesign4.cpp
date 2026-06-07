/**
 * @file FilterDesign4.cpp
 * @brief FilterDesign4 实现
 *
 * 实现数字滤波器设计：Parks-McClellan等波纹、WLS二阶约束、频率响应分析。
 */

#include "utils/signal209/FilterDesign4.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

FilterDesign4::FilterDesign4(QObject *parent) : QObject(parent) {}
FilterDesign4::~FilterDesign4() = default;

/* ---- Configuration ---- */

void FilterDesign4::setFilterOrder(int order) { m_order = qMax(1, order); }
void FilterDesign4::setFilterType(FilterType type) { m_type = type; }
void FilterDesign4::setBands(const QVector<BandSpec>& bands) { m_bands = bands; }

/* ---- Cosine basis ---- */

double FilterDesign4::cosineBasis(int k, double freq)
{
    return qCos(2.0 * M_PI * k * freq);
}

/* ---- Build frequency grid ---- */

void FilterDesign4::buildFreqGrid(int gridDensity, QVector<double>& freqs,
                                    QVector<double>& desired,
                                    QVector<double>& weights) const
{
    freqs.clear();
    desired.clear();
    weights.clear();

    if (m_bands.isEmpty()) {
        // Default: lowpass
        double cutoff = 0.25;
        double transWidth = 0.05;
        int gridPts = gridDensity * m_order;

        for (int i = 0; i <= gridPts; ++i) {
            double f = static_cast<double>(i) / gridPts * 0.5;
            if (f <= cutoff - transWidth) {
                freqs.append(f);
                desired.append(1.0);
                weights.append(1.0);
            } else if (f >= cutoff + transWidth && f < 0.5) {
                freqs.append(f);
                desired.append(0.0);
                weights.append(1.0);
            }
        }
        return;
    }

    int gridPts = gridDensity * m_order;
    double fMin = 1.0 / (gridPts * 4);

    for (const auto& band : m_bands) {
        double lo = qMax(fMin, band.lowFreq);
        double hi = qMin(0.499, band.highFreq);
        int pts = qMax(4, static_cast<int>(gridPts * (hi - lo) / 0.5));
        for (int i = 0; i <= pts; ++i) {
            double f = lo + (hi - lo) * i / pts;
            freqs.append(f);
            desired.append(band.gain);
            weights.append(band.weight);
        }
    }
}

/* ---- Lagrange interpolation ---- */

double FilterDesign4::lagrangeInterp(double freq, const QVector<double>& extFreqs,
                                       const QVector<double>& extVals) const
{
    int n = extFreqs.size();
    double result = 0.0;
    for (int i = 0; i < n; ++i) {
        double term = extVals[i];
        for (int j = 0; j < n; ++j) {
            if (j != i) {
                double denom = extFreqs[i] - extFreqs[j];
                if (qAbs(denom) < 1e-15) denom = 1e-15;
                term *= (qCos(2.0 * M_PI * freq) - qCos(2.0 * M_PI * extFreqs[j])) / denom;
            }
        }
        result += term;
    }
    return result;
}

/* ---- Remez exchange ---- */

QVector<double> FilterDesign4::remezExchange(int N, const QVector<double>& freqGrid,
                                               const QVector<double>& desiredGrid,
                                               const QVector<double>& weightGrid) const
{
    int M = N + 1;  // Number of extremals
    int gridSize = freqGrid.size();
    if (gridSize < M) return QVector<double>(N + 1, 0.0);

    // Initial guess: uniform extremals
    QVector<int> extIndices(M);
    for (int i = 0; i < M; ++i)
        extIndices[i] = i * (gridSize - 1) / (M - 1);

    for (int iter = 0; iter < 40; ++iter) {
        // Compute optimal equiripple on current extremals
        QVector<double> extFreqs(M), extDesired(M);
        for (int i = 0; i < M; ++i) {
            extFreqs[i] = freqGrid[extIndices[i]];
            extDesired[i] = desiredGrid[extIndices[i]];
        }

        // Solve for delta (equiripple error) and coefficients
        // Using Vandermonde-like system: sum(a_k * cos(2pi*k*fi)) + (-1)^i * delta/wi = Di
        // Simplified: compute via Lagrange interpolation
        QVector<double> err(gridSize);
        for (int i = 0; i < gridSize; ++i) {
            double interp = lagrangeInterp(freqGrid[i], extFreqs, extDesired);
            err[i] = (interp - desiredGrid[i]) * weightGrid[i];
        }

        // Find new extremals (maximum error locations)
        QVector<int> newExt;
        double maxErr = 0.0;
        for (int i = 0; i < gridSize; ++i)
            if (qAbs(err[i]) > maxErr) maxErr = qAbs(err[i]);

        // Find alternating peaks
        double threshold = maxErr * 0.9;
        int sign = 0;
        for (int i = 0; i < gridSize; ++i) {
            double e = err[i];
            int curSign = (e >= 0) ? 1 : -1;
            if (qAbs(e) >= threshold && curSign != sign) {
                newExt.append(i);
                sign = curSign;
                if (newExt.size() >= M) break;
            }
        }

        // Pad if needed
        while (newExt.size() < M)
            newExt.append(qMin(newExt.isEmpty() ? 0 : newExt.last() + 1, gridSize - 1));

        // Check convergence
        bool converged = true;
        for (int i = 0; i < M; ++i) {
            if (newExt[i] != extIndices[i]) { converged = false; break; }
        }
        extIndices = newExt;
        if (converged) break;
    }

    // Extract coefficients from final extremal set
    QVector<double> coeffs(N + 1, 0.0);
    for (int k = 0; k <= N; ++k) {
        double sum = 0.0;
        for (int i = 0; i < qMin(static_cast<int>(extIndices.size()), M); ++i) {
            double f = freqGrid[extIndices[i]];
            sum += desiredGrid[extIndices[i]] * cosineBasis(k, f);
        }
        coeffs[k] = sum / M;
    }

    return coeffs;
}

/* ---- Design equiripple ---- */

QVector<double> FilterDesign4::designEquiripple()
{
    QElapsedTimer timer;
    timer.start();

    QVector<double> freqs, desired, weights;
    buildFreqGrid(16, freqs, desired, weights);

    int N = m_order / 2;
    m_coeffs = remezExchange(N, freqs, desired, weights);

    m_stats.totalDesigns++;
    m_stats.filterOrder = m_order;
    m_stats.numBands = m_bands.size();
    m_stats.designTimeMs = timer.elapsed();
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalDesigns;
    emit designCompleted(m_order, m_bands.size(), timer.elapsed());

    return m_coeffs;
}

/* ---- Design WLS ---- */

QVector<double> FilterDesign4::designWLS()
{
    QElapsedTimer timer;
    timer.start();

    QVector<double> freqs, desired, weights;
    buildFreqGrid(16, freqs, desired, weights);

    int N = m_order / 2 + 1;
    int G = freqs.size();

    // Build normal equations: A^T W A c = A^T W d
    QVector<QVector<double>> ATA(N, QVector<double>(N, 0.0));
    QVector<double> ATd(N, 0.0);

    for (int g = 0; g < G; ++g) {
        double w = weights[g] * weights[g];  // WLS weight squared
        // Second-order constraint: add regularization
        w += 1e-6;  // Tikhonov regularization

        for (int k = 0; k < N; ++k) {
            double cosK = cosineBasis(k, freqs[g]);
            ATd[k] += w * cosK * desired[g];
            for (int l = 0; l < N; ++l) {
                ATA[k][l] += w * cosK * cosineBasis(l, freqs[g]);
            }
        }
    }

    // Solve via Gaussian elimination
    m_coeffs.resize(N, 0.0);
    // Forward elimination with partial pivoting
    for (int k = 0; k < N; ++k) {
        int pivot = k;
        for (int i = k + 1; i < N; ++i)
            if (qAbs(ATA[i][k]) > qAbs(ATA[pivot][k])) pivot = i;
        if (pivot != k) {
            std::swap(ATA[k], ATA[pivot]);
            std::swap(ATd[k], ATd[pivot]);
        }
        if (qAbs(ATA[k][k]) < 1e-15) continue;
        for (int i = k + 1; i < N; ++i) {
            double factor = ATA[i][k] / ATA[k][k];
            for (int j = k; j < N; ++j)
                ATA[i][j] -= factor * ATA[k][j];
            ATd[i] -= factor * ATd[k];
        }
    }
    // Back substitution
    for (int i = N - 1; i >= 0; --i) {
        double sum = ATd[i];
        for (int j = i + 1; j < N; ++j)
            sum -= ATA[i][j] * m_coeffs[j];
        m_coeffs[i] = (qAbs(ATA[i][i]) > 1e-15) ? sum / ATA[i][i] : 0.0;
    }

    m_stats.totalDesigns++;
    m_stats.filterOrder = m_order;
    m_stats.numBands = m_bands.size();
    m_stats.designTimeMs = timer.elapsed();
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalDesigns;
    emit designCompleted(m_order, m_bands.size(), timer.elapsed());

    return m_coeffs;
}

/* ---- Frequency response ---- */

QVector<QPair<double, double>> FilterDesign4::frequencyResponse(
    const QVector<double>& coeffs, const QVector<double>& freqs) const
{
    QVector<QPair<double, double>> response(freqs.size());
    for (int f = 0; f < freqs.size(); ++f) {
        double re = 0.0, im = 0.0;
        for (int k = 0; k < coeffs.size(); ++k) {
            double angle = -2.0 * M_PI * k * freqs[f];
            re += coeffs[k] * qCos(angle);
            im += coeffs[k] * qSin(angle);
        }
        response[f] = {re, im};
    }
    return response;
}

/* ---- Group delay ---- */

QVector<double> FilterDesign4::groupDelay(const QVector<double>& coeffs,
                                            const QVector<double>& freqs) const
{
    QVector<double> delay(freqs.size(), 0.0);
    double eps = 1e-6;

    for (int f = 0; f < freqs.size(); ++f) {
        auto resp1 = frequencyResponse(coeffs, {freqs[f] - eps});
        auto resp2 = frequencyResponse(coeffs, {freqs[f] + eps});

        if (resp1.size() > 0 && resp2.size() > 0) {
            double phase1 = qAtan2(resp1[0].second, resp1[0].first);
            double phase2 = qAtan2(resp2[0].second, resp2[0].first);
            delay[f] = -(phase2 - phase1) / (2.0 * eps * 2.0 * M_PI);
        }
    }
    return delay;
}

/* ---- Get coefficients ---- */

QVector<double> FilterDesign4::getCoefficients() const { return m_coeffs; }

/* ---- Reset ---- */

void FilterDesign4::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_coeffs.clear();
}
