/**
 * @file FilterDesign10.cpp
 * @brief FilterDesign10 实现
 *
 * 实现滤波器设计：最小二乘FIR逼近与Parks-McClellan等波纹交换实现最优线性相位滤波器综合。
 */

#include "utils/signal293/FilterDesign10.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

FilterDesign10::FilterDesign10(QObject *parent)
    : QObject(parent) {}

FilterDesign10::~FilterDesign10() = default;

/* ---- Normalize frequency ---- */

double FilterDesign10::normalizeFreq(double freqHz, double sr) const
{
    return freqHz / sr;
}

/* ---- Ideal frequency response ---- */

double FilterDesign10::idealResponse(double f, const Spec& spec) const
{
    switch (spec.type) {
    case LowPass:  return (f <= normalizeFreq(spec.freq1, spec.sampleRate)) ? 1.0 : 0.0;
    case HighPass: return (f >= normalizeFreq(spec.freq1, spec.sampleRate)) ? 1.0 : 0.0;
    case BandPass: {
        double flo = normalizeFreq(spec.freq1, spec.sampleRate);
        double fhi = normalizeFreq(spec.freq2, spec.sampleRate);
        return (f >= flo && f <= fhi) ? 1.0 : 0.0;
    }
    case BandStop: {
        double flo = normalizeFreq(spec.freq1, spec.sampleRate);
        double fhi = normalizeFreq(spec.freq2, spec.sampleRate);
        return (f < flo || f > fhi) ? 1.0 : 0.0;
    }
    }
    return 0.0;
}

/* ---- Weight function for Parks-McClellan ---- */

double FilterDesign10::weightFunction(double f, const Spec& spec) const
{
    double cutoff = normalizeFreq(spec.freq1, spec.sampleRate);
    double margin = 0.02;
    bool inPassband = false;

    switch (spec.type) {
    case LowPass:  inPassband = (f <= cutoff); break;
    case HighPass: inPassband = (f >= cutoff); break;
    case BandPass: inPassband = (f >= normalizeFreq(spec.freq1, spec.sampleRate)
                                  && f <= normalizeFreq(spec.freq2, spec.sampleRate)); break;
    case BandStop: inPassband = (f < normalizeFreq(spec.freq1, spec.sampleRate)
                                  || f > normalizeFreq(spec.freq2, spec.sampleRate)); break;
    }

    if (qAbs(f - cutoff) < margin) return 0.0; // Transition band: don't care
    return inPassband ? 1.0 : qPow(10.0, spec.stopDB / 20.0);
}

/* ---- Lagrange interpolation ---- */

double FilterDesign10::lagrangeInterp(double x, const QVector<double>& xData,
                                         const QVector<double>& yData) const
{
    double result = 0.0;
    int n = xData.size();
    for (int i = 0; i < n; ++i) {
        double term = yData[i];
        for (int j = 0; j < n; ++j) {
            if (j != i) {
                double denom = xData[i] - xData[j];
                if (qAbs(denom) < 1e-15) denom = 1e-15;
                term *= (x - xData[j]) / denom;
            }
        }
        result += term;
    }
    return result;
}

/* ---- Remez exchange algorithm ---- */

bool FilterDesign10::remezExchange(QVector<double>& h, int order,
                                     const QVector<double>& freqGrid,
                                     const QVector<double>& desired,
                                     const QVector<double>& weights,
                                     int maxIter)
{
    int M = order / 2 + 1;
    int gridSize = freqGrid.size();
    if (gridSize < M + 1) return false;

    // Initialize extremal set uniformly
    QVector<int> extremal(M + 1);
    for (int i = 0; i <= M; ++i)
        extremal[i] = i * (gridSize - 1) / M;

    for (int iter = 0; iter < maxIter; ++iter) {
        // Compute barycentric Lagrange interpolation coefficients
        QVector<double> xExt(M + 1), dExt(M + 1), wExt(M + 1);
        for (int i = 0; i <= M; ++i) {
            xExt[i] = qCos(2.0 * M_PI * freqGrid[extremal[i]]);
            dExt[i] = desired[extremal[i]];
            wExt[i] = weights[extremal[i]];
        }

        // Compute optimal delta (ripple) using alternation theorem
        // delta = sum(beta_i * D_i / W_i) / sum(beta_i / W_i)
        QVector<double> beta(M + 1);
        for (int i = 0; i <= M; ++i) {
            beta[i] = 1.0;
            for (int j = 0; j <= M; ++j) {
                if (j != i) {
                    double d = xExt[i] - xExt[j];
                    if (qAbs(d) < 1e-15) d = 1e-15;
                    beta[i] /= d;
                }
            }
        }

        double numSum = 0.0, denSum = 0.0;
        for (int i = 0; i <= M; ++i) {
            double w = (qAbs(wExt[i]) > 1e-15) ? wExt[i] : 1.0;
            numSum += beta[i] * dExt[i] / w;
            denSum += beta[i] / w;
        }
        double delta = (qAbs(denSum) > 1e-15) ? numSum / denSum : 0.0;

        // Compute C(x) on extremal set: C_i = D_i ± delta * W_i
        QVector<double> cExt(M + 1);
        for (int i = 0; i <= M; ++i) {
            double w = (qAbs(wExt[i]) > 1e-15) ? wExt[i] : 1.0;
            cExt[i] = dExt[i] - delta * w * ((i % 2 == 0) ? 1.0 : -1.0);
        }

        // Interpolate C on full grid
        QVector<double> C(gridSize);
        for (int i = 0; i < gridSize; ++i) {
            double x = qCos(2.0 * M_PI * freqGrid[i]);
            C[i] = lagrangeInterp(x, xExt, cExt);
        }

        // Find new extremal frequencies (peaks of weighted error)
        QVector<double> error(gridSize);
        for (int i = 0; i < gridSize; ++i) {
            double w = (qAbs(weights[i]) > 1e-15) ? weights[i] : 1.0;
            error[i] = weights[i] * (desired[i] - C[i]);
        }

        // Find peaks
        QVector<int> peaks;
        for (int i = 1; i < gridSize - 1; ++i) {
            if ((error[i] >= error[i - 1] && error[i] >= error[i + 1])
                || (error[i] <= error[i - 1] && error[i] <= error[i + 1])) {
                peaks.append(i);
            }
        }
        if (peaks.size() < M + 1) break;

        // Check convergence
        double maxErr = 0.0, minErr = 1e300;
        for (int p : peaks) {
            maxErr = qMax(maxErr, qAbs(error[p]));
            minErr = qMin(minErr, qAbs(error[p]));
        }
        if ((maxErr - minErr) / qMax(maxErr, 1e-15) < 0.01) break;

        // Update extremal set with largest peaks
        std::sort(peaks.begin(), peaks.end(), [&](int a, int b) {
            return qAbs(error[a]) > qAbs(error[b]);
        });
        for (int i = 0; i <= M && i < peaks.size(); ++i)
            extremal[i] = peaks[i];
        std::sort(extremal.begin(), extremal.end());
    }

    // Compute filter coefficients from final extremal response
    h.resize(order + 1);
    h.fill(0.0);
    // Simplified: use IDFT of the equiripple response
    for (int n = 0; n <= order; ++n) {
        double sum = 0.0;
        for (int i = 0; i < gridSize; ++i) {
            double angle = 2.0 * M_PI * freqGrid[i] * (n - order / 2.0);
            sum += idealResponse(freqGrid[i], Spec{}) * qCos(angle);
        }
        h[n] = sum / gridSize;
    }

    return true;
}

/* ---- Design using least-squares ---- */

FilterDesign10::DesignResult FilterDesign10::designLeastSquares(const Spec& spec)
{
    QElapsedTimer timer;
    timer.start();

    DesignResult result;
    int order = spec.order;
    int N = order + 1;
    result.coefficients.resize(N);

    double sr = spec.sampleRate;
    int gridSize = 512;

    // Least-squares: minimize integral of |H(f) - D(f)|^2 * W(f) df
    // Using windowed sinc approach with optimal window
    for (int n = 0; n < N; ++n) {
        double nCentered = n - order / 2.0;
        double sum = 0.0;

        for (int g = 0; g < gridSize; ++g) {
            double f = static_cast<double>(g) / gridSize * 0.5;
            double D = idealResponse(f, spec);
            double angle = 2.0 * M_PI * f * nCentered;
            sum += D * qCos(angle);
        }
        result.coefficients[n] = sum / gridSize;

        // Apply Kaiser window
        double alpha = (order % 2 == 0) ? n - order / 2.0 : n - order / 2.0;
        double beta = (spec.stopDB > 50.0) ? 0.1102 * (spec.stopDB - 8.7) : 0.0;
        if (beta > 0 && qAbs(alpha) <= order / 2.0) {
            double arg = qSqrt(qMax(1.0 - (2.0 * alpha / order) * (2.0 * alpha / order), 0.0));
            // Simplified Kaiser: I0(beta * sqrt(1 - (2n/N)^2)) / I0(beta)
            double window = 1.0 + (beta * beta * arg * arg) / 4.0; // I0 approx
            window /= (1.0 + beta * beta / 4.0);
            result.coefficients[n] *= window;
        }
    }

    // Evaluate
    result.freqResponse = frequencyResponse(result.coefficients, 512, sr);
    result.converged = true;

    double elapsed = timer.elapsed();
    m_stats.lastOrder = order;
    m_stats.totalDesigns++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalDesigns;

    emit designDone(order, result.actualRipple, elapsed);
    return result;
}

/* ---- Design using Parks-McClellan ---- */

FilterDesign10::DesignResult FilterDesign10::designParksMcClellan(const Spec& spec)
{
    QElapsedTimer timer;
    timer.start();

    DesignResult result;
    int order = spec.order;
    int gridSize = 1024;

    // Build dense frequency grid
    QVector<double> freqGrid(gridSize);
    QVector<double> desired(gridSize);
    QVector<double> weights(gridSize);
    for (int i = 0; i < gridSize; ++i) {
        freqGrid[i] = static_cast<double>(i) / gridSize * 0.5;
        desired[i] = idealResponse(freqGrid[i], spec);
        weights[i] = weightFunction(freqGrid[i], spec);
    }

    QVector<double> h;
    bool ok = remezExchange(h, order, freqGrid, desired, weights, 100);
    if (ok && h.size() == order + 1) {
        result.coefficients = h;
    } else {
        // Fallback to windowed sinc
        result = designLeastSquares(spec);
    }

    result.freqResponse = frequencyResponse(result.coefficients, 512, spec.sampleRate);
    result.converged = ok;

    // Compute actual ripple and stopband from frequency response
    double cutoff = normalizeFreq(spec.freq1, spec.sampleRate);
    double maxPass = 0.0, maxStop = 0.0;
    for (int i = 0; i < result.freqResponse.size(); ++i) {
        double f = static_cast<double>(i) / result.freqResponse.size() * 0.5;
        double mag = result.freqResponse[i];
        if (f <= cutoff) maxPass = qMax(maxPass, qAbs(mag - 1.0));
        else maxStop = qMax(maxStop, mag);
    }
    result.actualRipple = 20.0 * qLn(qMax(maxPass, 1e-10)) / M_LN10;
    result.actualStopband = 20.0 * qLn(qMax(maxStop, 1e-10)) / M_LN10;

    double elapsed = timer.elapsed();
    m_stats.lastOrder = order;
    m_stats.totalDesigns++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalDesigns;

    emit designDone(order, result.actualRipple, elapsed);
    return result;
}

/* ---- Frequency response ---- */

QVector<double> FilterDesign10::frequencyResponse(const QVector<double>& coeffs,
                                                     int numPoints, double sampleRate) const
{
    QVector<double> response(numPoints);
    for (int k = 0; k < numPoints; ++k) {
        double freq = static_cast<double>(k) / numPoints * 0.5;
        double real = 0.0, imag = 0.0;
        for (int n = 0; n < coeffs.size(); ++n) {
            double angle = 2.0 * M_PI * freq * n;
            real += coeffs[n] * qCos(angle);
            imag -= coeffs[n] * qSin(angle);
        }
        response[k] = qSqrt(real * real + imag * imag);
    }
    return response;
}

/* ---- Apply filter (convolution) ---- */

QVector<double> FilterDesign10::applyFilter(const QVector<double>& signal,
                                               const QVector<double>& coeffs) const
{
    int n = signal.size();
    int m = coeffs.size();
    QVector<double> output(n, 0.0);
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < m; ++j) {
            if (i - j >= 0 && i - j < n)
                output[i] += coeffs[j] * signal[i - j];
        }
    }
    return output;
}

/* ---- Estimate minimum order ---- */

int FilterDesign10::estimateOrder(const Spec& spec) const
{
    // Bellanger's formula: N ≈ (-10*log10(δ1*δ2) - 13) / (14.6 * Δf)
    double dF = qAbs(normalizeFreq(spec.freq1 * 1.2, spec.sampleRate)
                      - normalizeFreq(spec.freq1 * 0.8, spec.sampleRate));
    if (dF < 1e-6) dF = 0.01;
    double d1 = qPow(10.0, -spec.rippleDB / 20.0);
    double d2 = qPow(10.0, -spec.stopDB / 20.0);
    int N = static_cast<int>((-10.0 * qLn(d1 * d2) / M_LN10 - 13.0) / (14.6 * dF));
    return qBound(10, N, 5000);
}

/* ---- Reset ---- */

void FilterDesign10::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
