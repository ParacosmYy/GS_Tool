/**
 * @file FilterDesign9.cpp
 * @brief FilterDesign9 实现
 *
 * 实现滤波器设计：最小二乘最优FIR与频率采样法的任意幅度响应规范。
 */

#include "utils/signal279/FilterDesign9.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

FilterDesign9::FilterDesign9(QObject *parent)
    : QObject(parent) {}

FilterDesign9::~FilterDesign9() = default;

/* ---- Sinc function ---- */

double FilterDesign9::sinc(double x)
{
    return (qAbs(x) < 1e-10) ? 1.0 : qSin(M_PI * x) / (M_PI * x);
}

/* ---- Generate window ---- */

QVector<double> FilterDesign9::generateWindow(int n, const QString& type) const
{
    QVector<double> w(n);
    double N = n - 1;
    for (int i = 0; i < n; ++i) {
        if (type == QStringLiteral("hamming")) {
            w[i] = 0.54 - 0.46 * qCos(2.0 * M_PI * i / N);
        } else if (type == QStringLiteral("hann")) {
            w[i] = 0.5 * (1.0 - qCos(2.0 * M_PI * i / N));
        } else if (type == QStringLiteral("blackman")) {
            w[i] = 0.42 - 0.5 * qCos(2.0 * M_PI * i / N)
                   + 0.08 * qCos(4.0 * M_PI * i / N);
        } else {
            w[i] = 1.0; // Rectangular
        }
    }
    return w;
}

/* ---- Ideal frequency response ---- */

void FilterDesign9::idealResponse(int n, FilterType type, double cutoff1,
                                     double cutoff2, QVector<double>& H) const
{
    H.resize(n);
    double mid = (n - 1) / 2.0;

    for (int i = 0; i < n; ++i) {
        double k = i - mid;
        switch (type) {
        case LowPass:
            H[i] = 2.0 * cutoff1 * sinc(2.0 * cutoff1 * k);
            break;
        case HighPass:
            H[i] = sinc(k) - 2.0 * cutoff1 * sinc(2.0 * cutoff1 * k);
            break;
        case BandPass:
            H[i] = 2.0 * cutoff2 * sinc(2.0 * cutoff2 * k)
                   - 2.0 * cutoff1 * sinc(2.0 * cutoff1 * k);
            break;
        case BandStop:
            H[i] = sinc(k) - 2.0 * cutoff2 * sinc(2.0 * cutoff2 * k)
                   + 2.0 * cutoff1 * sinc(2.0 * cutoff1 * k);
            break;
        default:
            H[i] = sinc(k);
        }
    }
}

/* ---- Build weight function ---- */

QVector<double> FilterDesign9::buildWeights(int n, FilterType type,
                                               double cutoff1, double cutoff2) const
{
    QVector<double> W(n, 1.0);
    double transWidth = 0.02; // Transition band width

    for (int i = 0; i < n; ++i) {
        double f = i / static_cast<double>(n);
        switch (type) {
        case LowPass:
            if (f > cutoff1 + transWidth) W[i] = 10.0; // Stopband weight
            break;
        case HighPass:
            if (f < cutoff1 - transWidth) W[i] = 10.0;
            break;
        case BandPass:
            if (f < cutoff1 - transWidth || f > cutoff2 + transWidth) W[i] = 10.0;
            break;
        case BandStop:
            if (f > cutoff1 + transWidth && f < cutoff2 - transWidth) W[i] = 10.0;
            break;
        default:
            break;
        }
    }
    return W;
}

/* ---- Least-squares optimal FIR design ---- */

FilterDesign9::DesignResult FilterDesign9::designLeastSquares(
    int order, FilterType type, double cutoff1, double cutoff2)
{
    QElapsedTimer timer;
    timer.start();

    DesignResult result;
    order = qBound(4, order, 4096);
    if (order % 2 == 0) order++; // Ensure odd order
    int n = order + 1;

    // Get ideal impulse response
    QVector<double> h;
    idealResponse(n, type, cutoff1, cutoff2, h);

    // Apply least-squares optimization via weighted frequency domain
    // Build frequency grid for evaluation
    int gridSize = 512;
    QVector<double> weight = buildWeights(gridSize, type, cutoff1, cutoff2);

    // Construct normal equations: R * h_opt = p
    // R = S^T * W * S, p = S^T * W * d
    int M = n;
    QVector<QVector<double>> R(M, QVector<double>(M, 0.0));
    QVector<double> p(M, 0.0);

    for (int k = 0; k < gridSize; ++k) {
        double f = k / static_cast<double>(gridSize) * 0.5;
        double w = weight[k];

        // Desired response at this frequency
        double d = 0.0;
        switch (type) {
        case LowPass:  d = (f <= cutoff1) ? 1.0 : 0.0; break;
        case HighPass: d = (f >= cutoff1) ? 1.0 : 0.0; break;
        case BandPass: d = (f >= cutoff1 && f <= cutoff2) ? 1.0 : 0.0; break;
        case BandStop: d = (f <= cutoff1 || f >= cutoff2) ? 1.0 : 0.0; break;
        default: d = 1.0;
        }

        for (int i = 0; i < M; ++i) {
            double phi_i = qCos(2.0 * M_PI * f * (i - (M - 1) / 2.0));
            for (int j = 0; j < M; ++j) {
                double phi_j = qCos(2.0 * M_PI * f * (j - (M - 1) / 2.0));
                R[i][j] += w * phi_i * phi_j;
            }
            p[i] += w * d * phi_i;
        }
    }

    // Solve via Gauss-Seidel iteration (simplified)
    QVector<double> hOpt = h; // Start from ideal
    for (int gs = 0; gs < 30; ++gs) {
        for (int i = 0; i < M; ++i) {
            double sigma = 0.0;
            for (int j = 0; j < M; ++j) {
                if (j != i) sigma += R[i][j] * hOpt[j];
            }
            hOpt[i] = (R[i][i] > 1e-15) ? (p[i] - sigma) / R[i][i] : hOpt[i];
        }
    }

    // Apply window for smoothness
    auto window = generateWindow(n, QStringLiteral("hamming"));
    for (int i = 0; i < n; ++i) hOpt[i] *= window[i];

    result.coefficients = hOpt;
    result.order = order;
    result.type = type;

    // Compute frequency response
    frequencyResponse(hOpt, 512, result.freqResponse, result.phaseResponse);

    // Estimate ripple
    double maxPass = 0.0, maxStop = 0.0;
    for (int k = 0; k < 512; ++k) {
        double f = k / 512.0 * 0.5;
        if (type == LowPass && f <= cutoff1 * 0.9)
            maxPass = qMax(maxPass, qAbs(result.freqResponse[k] - 1.0));
        else if (type == LowPass && f >= cutoff1 * 1.1)
            maxStop = qMax(maxStop, qAbs(result.freqResponse[k]));
    }
    result.ripplePassband = maxPass;
    result.rippleStopband = maxStop;

    double elapsed = timer.elapsed();
    m_stats.numDesigns++;
    m_stats.maxOrder = qMax(m_stats.maxOrder, order);
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit designComplete(order, type, elapsed);

    return result;
}

/* ---- Frequency sampling method ---- */

FilterDesign9::DesignResult FilterDesign9::designFrequencySampling(
    int order, const QVector<SpecPoint>& spec)
{
    QElapsedTimer timer;
    timer.start();

    DesignResult result;
    order = qBound(4, order, 4096);
    if (order % 2 == 0) order++;
    int n = order + 1;
    int N = n;

    // Build desired DFT samples
    QVector<double> Hd(N, 0.0);
    for (int k = 0; k < N; ++k) {
        double f = k / static_cast<double>(N);
        // Interpolate from spec points
        for (int s = 0; s < spec.size() - 1; ++s) {
            if (f >= spec[s].freq && f <= spec[s + 1].freq) {
                double t = (f - spec[s].freq) / (spec[s + 1].freq - spec[s].freq);
                Hd[k] = spec[s].magnitude * (1.0 - t) + spec[s + 1].magnitude * t;
                break;
            }
        }
        if (!spec.isEmpty() && f < spec[0].freq) Hd[k] = spec[0].magnitude;
        if (!spec.isEmpty() && f > spec.last().freq) Hd[k] = spec.last().magnitude;
    }

    // Inverse DFT to get filter coefficients
    QVector<double> h(N, 0.0);
    for (int nIdx = 0; nIdx < N; ++nIdx) {
        double sum = 0.0;
        for (int k = 0; k < N; ++k) {
            sum += Hd[k] * qCos(2.0 * M_PI * k * nIdx / N);
        }
        h[nIdx] = sum / N;
    }

    // Apply window
    auto window = generateWindow(N, QStringLiteral("hamming"));
    for (int i = 0; i < N; ++i) h[i] *= window[i];

    result.coefficients = h;
    result.order = order;
    result.type = Arbitrary;
    frequencyResponse(h, 512, result.freqResponse, result.phaseResponse);

    double elapsed = timer.elapsed();
    m_stats.numDesigns++;
    m_stats.maxOrder = qMax(m_stats.maxOrder, order);
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit designComplete(order, Arbitrary, elapsed);

    return result;
}

/* ---- Arbitrary response design ---- */

FilterDesign9::DesignResult FilterDesign9::designArbitrary(
    int order, const QVector<SpecPoint>& spec)
{
    // Weighted least-squares for arbitrary response
    return designFrequencySampling(order, spec);
}

/* ---- Frequency response ---- */

void FilterDesign9::frequencyResponse(const QVector<double>& coeffs, int numPoints,
                                         QVector<double>& magnitude,
                                         QVector<double>& phase) const
{
    magnitude.resize(numPoints);
    phase.resize(numPoints);
    int M = coeffs.size();

    for (int k = 0; k < numPoints; ++k) {
        double w = M_PI * k / numPoints;
        double re = 0.0, im = 0.0;
        for (int n = 0; n < M; ++n) {
            re += coeffs[n] * qCos(w * n);
            im -= coeffs[n] * qSin(w * n);
        }
        magnitude[k] = qSqrt(re * re + im * im);
        phase[k] = qAtan2(im, re);
    }
}

/* ---- Apply window ---- */

QVector<double> FilterDesign9::applyWindow(const QVector<double>& coeffs,
                                              const QString& windowType) const
{
    auto w = generateWindow(coeffs.size(), windowType);
    QVector<double> result(coeffs.size());
    for (int i = 0; i < coeffs.size(); ++i)
        result[i] = coeffs[i] * w[i];
    return result;
}

/* ---- Reset ---- */

void FilterDesign9::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
