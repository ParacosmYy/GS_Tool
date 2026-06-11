/**
 * @file EnvelopeDetector11.cpp
 * @brief EnvelopeDetector11 实现
 *
 * 实现包络检测器：Hilbert-Huang经验模态分解与瞬时能量跟踪实现非平稳信号分析。
 */

#include "utils/signal300/EnvelopeDetector11.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

EnvelopeDetector11::EnvelopeDetector11(QObject *parent)
    : QObject(parent) {}

EnvelopeDetector11::~EnvelopeDetector11() = default;

/* ---- Configuration ---- */

void EnvelopeDetector11::setMaxIMFs(int maxIMFs) { m_maxIMFs = qBound(1, maxIMFs, 50); }
void EnvelopeDetector11::setMaxSiftIterations(int maxIter) { m_maxSiftIter = qBound(5, maxIter, 1000); }
void EnvelopeDetector11::setSiftTolerance(double tol) { m_siftTol = qBound(0.001, tol, 1.0); }

/* ---- Find local maxima ---- */

QVector<int> EnvelopeDetector11::findMaxima(const QVector<double>& x) const
{
    QVector<int> peaks;
    int n = x.size();
    if (n < 3) return peaks;

    // Include endpoints as extrema for envelope computation
    peaks.append(0);
    for (int i = 1; i < n - 1; ++i) {
        if (x[i] >= x[i - 1] && x[i] >= x[i + 1])
            peaks.append(i);
    }
    peaks.append(n - 1);
    return peaks;
}

/* ---- Find local minima ---- */

QVector<int> EnvelopeDetector11::findMinima(const QVector<double>& x) const
{
    QVector<int> troughs;
    int n = x.size();
    if (n < 3) return troughs;

    troughs.append(0);
    for (int i = 1; i < n - 1; ++i) {
        if (x[i] <= x[i - 1] && x[i] <= x[i + 1])
            troughs.append(i);
    }
    troughs.append(n - 1);
    return troughs;
}

/* ---- Cubic spline interpolation (natural spline) ---- */

QVector<double> EnvelopeDetector11::splineInterpolate(
    const QVector<double>& xData, const QVector<double>& yData,
    const QVector<double>& xEval) const
{
    int n = xData.size();
    int m = xEval.size();
    if (n < 2) return QVector<double>(m, (n > 0) ? yData[0] : 0.0);

    // Solve tridiagonal system for second derivatives
    QVector<double> h(n - 1);
    for (int i = 0; i < n - 1; ++i)
        h[i] = xData[i + 1] - xData[i];

    QVector<double> alpha(n, 0.0);
    for (int i = 1; i < n - 1; ++i)
        alpha[i] = 3.0 * ((yData[i + 1] - yData[i]) / h[i] -
                           (yData[i] - yData[i - 1]) / h[i - 1]);

    // Thomas algorithm
    QVector<double> c(n, 0.0), l(n, 0.0), mu(n, 0.0), z(n, 0.0);
    l[0] = 1.0;
    for (int i = 1; i < n - 1; ++i) {
        l[i] = 2.0 * (xData[i + 1] - xData[i - 1]) - h[i - 1] * mu[i - 1];
        if (qAbs(l[i]) < 1e-15) l[i] = 1e-15;
        mu[i] = h[i] / l[i];
        z[i] = (alpha[i] - h[i - 1] * z[i - 1]) / l[i];
    }

    QVector<double> b(n, 0.0), d(n, 0.0);
    for (int j = n - 2; j >= 0; --j) {
        c[j] = z[j] - mu[j] * c[j + 1];
        b[j] = (yData[j + 1] - yData[j]) / h[j] - h[j] * (c[j + 1] + 2.0 * c[j]) / 3.0;
        d[j] = (c[j + 1] - c[j]) / (3.0 * h[j]);
    }

    // Evaluate spline
    QVector<double> result(m);
    for (int i = 0; i < m; ++i) {
        double x = xEval[i];
        // Find interval
        int idx = 0;
        for (int j = 0; j < n - 1; ++j) {
            if (x >= xData[j]) idx = j;
        }
        idx = qMin(idx, n - 2);

        double dx = x - xData[idx];
        result[i] = yData[idx] + b[idx] * dx + c[idx] * dx * dx + d[idx] * dx * dx * dx;
    }
    return result;
}

/* ---- Sift one IMF ---- */

QVector<double> EnvelopeDetector11::sift(const QVector<double>& signal) const
{
    int n = signal.size();
    QVector<double> h = signal;

    for (int iter = 0; iter < m_maxSiftIter; ++iter) {
        auto maxIdx = findMaxima(h);
        auto minIdx = findMinima(h);

        if (maxIdx.size() < 2 || minIdx.size() < 2) break;

        // Extract x/y for spline
        QVector<double> maxX(maxIdx.size()), maxY(maxIdx.size());
        QVector<double> minX(minIdx.size()), minY(minIdx.size());
        QVector<double> xAll(n);
        for (int i = 0; i < n; ++i) xAll[i] = i;

        for (int i = 0; i < maxIdx.size(); ++i) {
            maxX[i] = maxIdx[i]; maxY[i] = h[maxIdx[i]];
        }
        for (int i = 0; i < minIdx.size(); ++i) {
            minX[i] = minIdx[i]; minY[i] = h[minIdx[i]];
        }

        auto upperEnv = splineInterpolate(maxX, maxY, xAll);
        auto lowerEnv = splineInterpolate(minX, minY, xAll);

        // Compute mean envelope
        double maxDev = 0.0;
        QVector<double> newH(n);
        for (int i = 0; i < n; ++i) {
            double mean = (upperEnv[i] + lowerEnv[i]) / 2.0;
            newH[i] = h[i] - mean;
            double localDev = qAbs(newH[i] - h[i] + mean);
            double localMean = qAbs(upperEnv[i] - lowerEnv[i]) / 2.0;
            if (localMean > 1e-10)
                maxDev = qMax(maxDev, localDev / localMean);
        }

        h = newH;

        if (maxDev < m_siftTol) break;
    }
    return h;
}

/* ---- Compute instantaneous properties ---- */

void EnvelopeDetector11::computeInstantaneous(IMF& imf) const
{
    int n = imf.signal.size();
    imf.instantaneousAmp.resize(n);
    imf.instantaneousFreq.resize(n);

    // Approximate analytic signal via simple quadrature (discrete Hilbert)
    // Using centered difference for phase derivative
    double energySum = 0.0;
    double freqSum = 0.0;
    int freqCount = 0;

    for (int i = 0; i < n; ++i) {
        // Approximate envelope via local RMS
        int halfW = qMin(5, qMin(i, n - 1 - i));
        double rms = 0.0;
        for (int j = i - halfW; j <= i + halfW; ++j) {
            if (j >= 0 && j < n)
                rms += imf.signal[j] * imf.signal[j];
        }
        rms = qSqrt(rms / (2 * halfW + 1));
        imf.instantaneousAmp[i] = qSqrt(2.0) * rms;
        energySum += imf.instantaneousAmp[i] * imf.instantaneousAmp[i];

        // Instantaneous frequency via phase difference
        if (i > 0 && i < n - 1) {
            double dy = imf.signal[i + 1] - imf.signal[i - 1];
            double y = imf.signal[i];
            double amp2 = y * y + (dy / 2.0) * (dy / 2.0);
            if (amp2 > 1e-20) {
                double freq = qAbs(dy) / (2.0 * M_PI * qSqrt(amp2));
                imf.instantaneousFreq[i] = freq;
                freqSum += freq;
                freqCount++;
            } else {
                imf.instantaneousFreq[i] = 0.0;
            }
        } else {
            imf.instantaneousFreq[i] = 0.0;
        }
    }

    imf.energy = energySum / n;
    imf.meanFreq = (freqCount > 0) ? freqSum / freqCount : 0.0;
}

/* ---- EMD decomposition ---- */

EnvelopeDetector11::EMDResult EnvelopeDetector11::decompose(
    const QVector<double>& input) const
{
    EMDResult result;
    int n = input.size();
    if (n < 4) return result;

    QVector<double> residual = input;

    for (int k = 0; k < m_maxIMFs; ++k) {
        auto imfSignal = sift(residual);
        int nZeroCross = 0;
        int nExtrema = 0;
        for (int i = 1; i < n; ++i) {
            if (imfSignal[i - 1] * imfSignal[i] < 0) nZeroCross++;
        }
        nExtrema = findMaxima(imfSignal).size() + findMinima(imfSignal).size() - 4;

        // Check if IMF criteria are approximately met
        double residualEnergy = 0.0;
        for (int i = 0; i < n; ++i)
            residualEnergy += imfSignal[i] * imfSignal[i];

        if (residualEnergy < 1e-20) break;

        IMF imf;
        imf.signal = imfSignal;
        computeInstantaneous(imf);
        result.imfs.append(imf);

        // Update residual
        for (int i = 0; i < n; ++i)
            residual[i] -= imfSignal[i];

        // Check if residual is monotonic (trend)
        bool monotonic = true;
        int dir = 0;
        for (int i = 1; i < n; ++i) {
            int d = (residual[i] > residual[i - 1]) ? 1 : -1;
            if (d != 0) {
                if (dir != 0 && d != dir) { monotonic = false; break; }
                dir = d;
            }
        }
        if (monotonic) break;
    }

    result.residual = residual;
    result.numIMFs = result.imfs.size();
    return result;
}

/* ---- Main detect ---- */

EnvelopeDetector11::EnvelopeResult EnvelopeDetector11::detect(
    const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    EnvelopeResult result;
    int n = input.size();
    if (n < 4) return result;

    // Perform EMD
    result.emdResult = decompose(input);

    // Compute envelope from IMF amplitudes
    result.upperEnvelope.resize(n);
    result.lowerEnvelope.resize(n);
    result.instantaneousEnergy.resize(n);

    for (int i = 0; i < n; ++i) {
        double envSum = 0.0;
        for (auto& imf : result.emdResult.imfs)
            envSum += imf.instantaneousAmp[i];
        result.upperEnvelope[i] = envSum + result.emdResult.residual[i];
        result.lowerEnvelope[i] = -envSum + result.emdResult.residual[i];
        result.instantaneousEnergy[i] = envSum * envSum;
    }

    // Peak and RMS of envelope
    double peakEnv = 0.0, rmsSum = 0.0;
    for (int i = 0; i < n; ++i) {
        peakEnv = qMax(peakEnv, result.upperEnvelope[i]);
        rmsSum += result.upperEnvelope[i] * result.upperEnvelope[i];
    }
    result.peakEnvelope = peakEnv;
    result.rmsEnvelope = qSqrt(rmsSum / n);

    m_stats.totalDetections++;
    m_stats.numIMFs = result.emdResult.numIMFs;

    double elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalDetections;

    emit detectionDone(n, result.emdResult.numIMFs, elapsed);
    return result;
}

/* ---- Reset ---- */

void EnvelopeDetector11::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
