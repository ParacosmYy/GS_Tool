/**
 * @file NoiseGate7.cpp
 * @brief NoiseGate7 实现
 *
 * 实现噪声门：信号直方图自适应阈值估计与迟滞控制开关。
 */

#include "utils/dsp247/NoiseGate7.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

NoiseGate7::NoiseGate7(QObject *parent) : QObject(parent) {}
NoiseGate7::~NoiseGate7() = default;

/* ---- Configuration ---- */

void NoiseGate7::setOpenThreshold(double t) { m_openThreshold = qMax(0.0, t); }
void NoiseGate7::setHysteresis(double h) { m_hysteresis = qMax(0.0, h); }
void NoiseGate7::setAttack(int s) { m_attack = qMax(1, s); }
void NoiseGate7::setRelease(int s) { m_release = qMax(1, s); }
void NoiseGate7::setHold(int s) { m_hold = qMax(0, s); }
void NoiseGate7::setClosedGain(double g) { m_closedGain = qBound(0.0, g, 1.0); }

/* ---- Smooth coefficient ---- */

double NoiseGate7::smoothCoeff(int timeSamples)
{
    if (timeSamples <= 0) return 1.0;
    return 1.0 - qExp(-1.0 / timeSamples);
}

/* ---- Build signal histogram ---- */

QVector<int> NoiseGate7::buildHistogram(const QVector<double>& input,
                                          int numBins, double& minVal,
                                          double& maxVal) const
{
    minVal = std::numeric_limits<double>::max();
    maxVal = -std::numeric_limits<double>::max();
    for (double v : input) {
        double absV = qAbs(v);
        if (absV < minVal) minVal = absV;
        if (absV > maxVal) maxVal = absV;
    }
    if (maxVal <= minVal) maxVal = minVal + 1e-6;

    double range = maxVal - minVal;
    QVector<int> bins(numBins, 0);
    for (double v : input) {
        double absV = qAbs(v);
        int bin = static_cast<int>((absV - minVal) / range * (numBins - 1));
        bin = qBound(0, bin, numBins - 1);
        bins[bin]++;
    }
    return bins;
}

/* ---- Find valley (noise floor boundary) in histogram ---- */

double NoiseGate7::findHistogramValley(const QVector<int>& bins,
                                         int numBins, double minVal,
                                         double maxVal) const
{
    // Find the first peak (noise) then the valley after it
    int numBins_ = bins.size();
    int peakIdx = 0;
    int peakCount = 0;
    for (int i = 0; i < numBins_; ++i) {
        if (bins[i] > peakCount) {
            peakCount = bins[i];
            peakIdx = i;
        }
    }

    // Find first valley after peak (where count rises again or stays low)
    int valleyIdx = peakIdx;
    int valleyCount = peakCount;
    for (int i = peakIdx + 1; i < numBins_; ++i) {
        if (bins[i] < valleyCount) {
            valleyCount = bins[i];
            valleyIdx = i;
        } else if (bins[i] > valleyCount * 2) {
            break;  // Rising signal region
        }
    }

    double range = maxVal - minVal;
    return minVal + (valleyIdx + 0.5) / numBins * range;
}

/* ---- Auto-estimate threshold ---- */

double NoiseGate7::estimateThreshold(const QVector<double>& input) const
{
    if (input.isEmpty()) return 0.01;
    double minV, maxV;
    auto bins = buildHistogram(input, 256, minV, maxV);
    return findHistogramValley(bins, 256, minV, maxV);
}

/* ---- Process signal ---- */

QVector<double> NoiseGate7::process(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    int n = input.size();
    QVector<double> output(n, 0.0);
    if (n == 0) return output;

    // Auto-estimate threshold if not set
    double openThresh = m_openThreshold;
    if (openThresh <= 0.0)
        openThresh = estimateThreshold(input);
    double closeThresh = qMax(0.0, openThresh - m_hysteresis);

    double attackCoeff = smoothCoeff(m_attack);
    double releaseCoeff = smoothCoeff(m_release);
    double envelope = 0.0;
    bool gateOpen = false;
    int holdCounter = 0;
    double currentGain = m_closedGain;
    int openings = 0;

    for (int i = 0; i < n; ++i) {
        double absSample = qAbs(input[i]);

        // Envelope follower (peak detection with separate attack/release)
        if (absSample > envelope)
            envelope += attackCoeff * (absSample - envelope);
        else
            envelope += releaseCoeff * (absSample - envelope);

        // Hysteresis gate logic
        if (!gateOpen) {
            if (envelope >= openThresh) {
                gateOpen = true;
                holdCounter = m_hold;
                openings++;
            }
        } else {
            if (holdCounter > 0) {
                holdCounter--;
            } else if (envelope < closeThresh) {
                gateOpen = false;
            }
        }

        // Smooth gain transition
        double targetGain = gateOpen ? 1.0 : m_closedGain;
        double gainCoeff = gateOpen ? attackCoeff : releaseCoeff;
        currentGain += gainCoeff * (targetGain - currentGain);

        output[i] = input[i] * currentGain;
    }

    m_stats.inputLength = n;
    m_stats.openThreshold = openThresh;
    m_stats.closeThreshold = closeThresh;
    m_stats.numOpenings = openings;
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit processingCompleted(n, openThresh, timer.elapsed());
    return output;
}

/* ---- Reset ---- */

void NoiseGate7::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
