/**
 * @file FilterDesign6.cpp
 * @brief FilterDesign6 实现
 *
 * 实现FIR滤波器设计：Parks-McClellan等波纹与加权切比雪夫逼近。
 */

#include "utils/signal237/FilterDesign6.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

FilterDesign6::FilterDesign6(QObject *parent) : QObject(parent) {}
FilterDesign6::~FilterDesign6() = default;

/* ---- Configuration ---- */

void FilterDesign6::setFilterOrder(int order) { m_filterOrder = qMax(2, order); }
void FilterDesign6::setPassbandRipple(double db) { m_passbandRipple = qMax(0.001, db); }
void FilterDesign6::setStopbandAttenuation(double db) { m_stopbandAttenuation = qMax(1.0, db); }
void FilterDesign6::setMaxIterations(int iter) { m_maxIterations = qMax(5, iter); }

/* ---- Sinc function ---- */

double FilterDesign6::sinc(double x)
{
    if (qAbs(x) < 1e-10) return 1.0;
    return qSin(M_PI * x) / (M_PI * x);
}

/* ---- Lagrange weight for Remez ---- */

double FilterDesign6::lagrangeWeight(int k, const QVector<double>& extFreqs, int numExtrema) const
{
    double w = 1.0;
    for (int i = 0; i < numExtrema; ++i) {
        if (i != k) {
            double denom = qCos(M_PI * extFreqs[k]) - qCos(M_PI * extFreqs[i]);
            if (qAbs(denom) > 1e-12)
                w *= 1.0 / denom;
        }
    }
    return w;
}

/* ---- Evaluate response ---- */

double FilterDesign6::evaluateResponse(double freq, const QVector<double>& coeffs) const
{
    int N = coeffs.size();
    double H = 0.0;
    for (int k = 0; k < N; ++k)
        H += coeffs[k] * qCos(2.0 * M_PI * freq * (k - N / 2));
    return H;
}

/* ---- Remez exchange algorithm ---- */

QVector<double> FilterDesign6::remezExchange(const QVector<double>& freqGrid,
                                              const QVector<double>& desiredGrid,
                                              const QVector<double>& weightGrid,
                                              int numTaps)
{
    int M = numTaps / 2 + 1;  // Number of cosine coefficients
    int numExtrema = M + 1;

    // Initialize extremal frequencies uniformly
    QVector<double> extFreqs(numExtrema);
    int gridSize = freqGrid.size();
    for (int i = 0; i < numExtrema; ++i) {
        int idx = i * (gridSize - 1) / qMax(1, numExtrema - 1);
        extFreqs[i] = freqGrid[qBound(0, idx, gridSize - 1)];
    }

    for (int iter = 0; iter < m_maxIterations; ++iter) {
        // Compute Lagrange interpolation weights
        double sumW = 0.0;
        QVector<double> beta(numExtrema);
        for (int k = 0; k < numExtrema; ++k) {
            beta[k] = lagrangeWeight(k, extFreqs, numExtrema);
            sumW += beta[k];
        }

        // Compute delta (deviation)
        double delta = 0.0;
        for (int k = 0; k < numExtrema; ++k) {
            int idx = 0;
            for (int j = 0; j < gridSize; ++j) {
                if (qAbs(freqGrid[j] - extFreqs[k]) < 1e-10) { idx = j; break; }
            }
            double sign = (k % 2 == 0) ? 1.0 : -1.0;
            delta += beta[k] * desiredGrid[idx];
        }
        if (qAbs(sumW) > 1e-15) delta /= sumW;

        emit remezIteration(iter, qAbs(delta));

        // Compute coefficients using interpolation at extremal frequencies
        // C(k) = D(Fk) - sign * delta / W(Fk)
        QVector<double> C(numExtrema);
        for (int k = 0; k < numExtrema; ++k) {
            int idx = 0;
            for (int j = 0; j < gridSize; ++j) {
                if (qAbs(freqGrid[j] - extFreqs[k]) < 1e-10) { idx = j; break; }
            }
            double sign = (k % 2 == 0) ? 1.0 : -1.0;
            C[k] = desiredGrid[idx] - sign * delta / weightGrid[idx];
        }

        // Evaluate error function on grid to find new extrema
        QVector<double> errors(gridSize);
        for (int i = 0; i < gridSize; ++i) {
            // Interpolate response at grid point using Lagrange
            double H = 0.0;
            for (int k = 0; k < numExtrema; ++k) {
                if (qAbs(beta[k]) > 1e-15) {
                    double w = lagrangeWeight(k, extFreqs, numExtrema);
                    double denom = qCos(M_PI * freqGrid[i]) - qCos(M_PI * extFreqs[k]);
                    if (qAbs(denom) > 1e-12)
                        H += C[k] * w / denom;
                }
            }
            errors[i] = (H - desiredGrid[i]) * weightGrid[i];
        }

        // Find new extremal points (alternation theorem)
        QVector<int> peaks;
        for (int i = 1; i < gridSize - 1; ++i) {
            if ((errors[i] >= errors[i - 1] && errors[i] >= errors[i + 1]) ||
                (errors[i] <= errors[i - 1] && errors[i] <= errors[i + 1])) {
                peaks.append(i);
            }
        }

        // Ensure we have numExtrema peaks
        if (peaks.size() >= numExtrema) {
            // Keep the largest magnitude peaks
            std::sort(peaks.begin(), peaks.end(), [&errors](int a, int b) {
                return qAbs(errors[a]) > qAbs(errors[b]);
            });
            peaks.resize(numExtrema);
            std::sort(peaks.begin(), peaks.end());
        }

        QVector<double> newExtFreqs(numExtrema);
        for (int k = 0; k < qMin(numExtrema, peaks.size()); ++k)
            newExtFreqs[k] = freqGrid[peaks[k]];
        // Fill remaining
        for (int k = peaks.size(); k < numExtrema; ++k)
            newExtFreqs[k] = extFreqs[k];

        extFreqs = newExtFreqs;
    }

    // Extract final coefficients from extremal frequencies
    // Use IDFT approach on the computed C values
    int N = numTaps;
    QVector<double> h(N, 0.0);
    double mid = (N - 1) / 2.0;
    for (int n = 0; n < N; ++n) {
        double sum = 0.0;
        for (int k = 0; k < numExtrema; ++k) {
            int idx = 0;
            for (int j = 0; j < gridSize; ++j) {
                if (qAbs(freqGrid[j] - extFreqs[k]) < 1e-10) { idx = j; break; }
            }
            sum += desiredGrid[idx] * qCos(2.0 * M_PI * extFreqs[k] * (n - mid));
        }
        h[n] = sum / numExtrema;
    }
    return h;
}

/* ---- Design ---- */

FilterDesign6::DesignResult FilterDesign6::design(const QVector<BandSpec>& bands, FilterType type)
{
    QElapsedTimer timer;
    timer.start();

    DesignResult result;
    int N = m_filterOrder + 1;

    if (bands.isEmpty()) {
        result.coefficients = designWindowedFIR(N, 0.5, type);
        result.filterOrder = m_filterOrder;
        return result;
    }

    // Build dense frequency grid
    int gridSize = 512;
    QVector<double> freqGrid, desiredGrid, weightGrid;

    for (int i = 0; i <= gridSize; ++i) {
        double f = static_cast<double>(i) / gridSize * 0.5;  // 0 to 0.5 (normalized)
        bool inBand = false;
        for (const auto& band : bands) {
            if (f >= band.lowFreq && f <= band.highFreq) {
                freqGrid.append(f);
                desiredGrid.append(band.isPassband ? 1.0 : 0.0);
                weightGrid.append(band.weight);
                inBand = true;
                break;
            }
        }
    }

    if (freqGrid.size() < N + 2) {
        result.coefficients = designWindowedFIR(N, 0.5, type);
        result.filterOrder = m_filterOrder;
        return result;
    }

    result.coefficients = remezExchange(freqGrid, desiredGrid, weightGrid, N);
    result.filterOrder = m_filterOrder;
    result.iterationsUsed = m_maxIterations;

    // Compute actual ripple from coefficients
    double maxPass = 0.0, maxStop = 0.0;
    for (int i = 0; i < freqGrid.size(); ++i) {
        double H = evaluateResponse(freqGrid[i], result.coefficients);
        double err = qAbs(H - desiredGrid[i]);
        if (desiredGrid[i] > 0.5) maxPass = qMax(maxPass, err);
        else maxStop = qMax(maxStop, err);
    }
    result.actualRippleDb = (maxPass > 0) ? 20.0 * qLn((1.0 + maxPass) / (1.0 - maxPass)) / M_LN10 : 0.0;
    result.actualStopbandDb = (maxStop > 0) ? -20.0 * qLn(maxStop) / M_LN10 : 120.0;

    m_stats.lastFilterOrder = m_filterOrder;
    m_stats.numDesigns++;
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit designCompleted(m_filterOrder, result.actualRippleDb, timer.elapsed());
    return result;
}

/* ---- Frequency response ---- */

QVector<double> FilterDesign6::frequencyResponse(const QVector<double>& coeffs,
                                                   const QVector<double>& freqs) const
{
    QVector<double> H(freqs.size());
    int N = coeffs.size();
    for (int i = 0; i < freqs.size(); ++i) {
        double re = 0.0;
        for (int k = 0; k < N; ++k)
            re += coeffs[k] * qCos(2.0 * M_PI * freqs[i] * k);
        H[i] = re;
    }
    return H;
}

/* ---- Windowed FIR fallback ---- */

QVector<double> FilterDesign6::designWindowedFIR(int taps, double cutoff, FilterType type) const
{
    QVector<double> h(taps, 0.0);
    double mid = (taps - 1) / 2.0;

    for (int n = 0; n < taps; ++n) {
        double x = n - mid;
        if (type == LowPass) {
            h[n] = 2.0 * cutoff * sinc(2.0 * cutoff * x);
        } else if (type == HighPass) {
            h[n] = sinc(x) - 2.0 * cutoff * sinc(2.0 * cutoff * x);
        }
        // Hamming window
        double w = 0.54 - 0.46 * qCos(2.0 * M_PI * n / (taps - 1));
        h[n] *= w;
    }
    return h;
}

/* ---- Reset ---- */

void FilterDesign6::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
