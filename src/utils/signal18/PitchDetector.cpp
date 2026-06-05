/**
 * @file PitchDetector.cpp
 * @brief 音高检测器实现
 */

#include "PitchDetector.h"
#include <QElapsedTimer>
#include <cmath>
#include <algorithm>
#include <limits>

PitchDetector::PitchDetector(double sampleRate, double minFreq,
                             double maxFreq, QObject* parent)
    : QObject(parent)
    , m_sampleRate(sampleRate)
    , m_minFreq(minFreq)
    , m_maxFreq(maxFreq)
    , m_timeSum(0.0)
{
}

PitchDetector::FrameResult PitchDetector::detect(const QVector<double>& frame,
                                                  Method method)
{
    QElapsedTimer timer;
    timer.start();

    FrameResult result;
    result.rms = computeRMS(frame);

    /* 无声判断 */
    if (result.rms < 1e-6) {
        result.frequency = 0.0;
        result.confidence = 0.0;
        result.voiced = false;

        m_stats.totalFrames++;
        m_stats.totalUnvoiced++;
        m_timeSum += timer.elapsed();
        m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalFrames;
        return result;
    }

    double freq = 0.0;
    double confidence = 0.0;

    if (method == YinMethod || method == Hybrid) {
        /* YIN算法 */
        auto diff = yinDifference(frame);
        auto cmnd = yinCMND(diff);

        /* 找第一个低于阈值的谷 */
        int minTau = qMax(1, static_cast<int>(m_sampleRate / m_maxFreq));
        int maxTau = qMin(static_cast<int>(m_sampleRate / m_minFreq),
                          cmnd.size() - 1);

        int bestTau = -1;
        double bestVal = 1.0;
        for (int tau = minTau; tau <= maxTau; ++tau) {
            if (cmnd[tau] < m_threshold && cmnd[tau] < bestVal) {
                bestVal = cmnd[tau];
                bestTau = tau;
            }
        }

        /* 如果没找到低于阈值的,找全局最小 */
        if (bestTau < 0) {
            for (int tau = minTau; tau <= maxTau; ++tau) {
                if (cmnd[tau] < bestVal) {
                    bestVal = cmnd[tau];
                    bestTau = tau;
                }
            }
        }

        if (bestTau > 0) {
            double refinedTau = parabolicRefine(cmnd, bestTau);
            if (refinedTau > 0)
                freq = m_sampleRate / refinedTau;
            confidence = 1.0 - bestVal;
        }
    }

    if (method == Autocorrelation || (method == Hybrid && freq <= 0)) {
        /* 自相关法 */
        auto acf = autocorrelation(frame);
        int minLag = qMax(1, static_cast<int>(m_sampleRate / m_maxFreq));
        int maxLag = qMin(static_cast<int>(m_sampleRate / m_minFreq),
                          acf.size() / 2);

        /* 找第一个峰 */
        int bestLag = -1;
        double bestAcf = -1.0;
        bool inPeak = false;

        for (int lag = minLag; lag <= maxLag; ++lag) {
            if (acf[lag] > 0.0 && !inPeak) {
                inPeak = true;
            }
            if (inPeak && acf[lag] > bestAcf) {
                bestAcf = acf[lag];
                bestLag = lag;
            }
            if (inPeak && acf[lag] < bestAcf * 0.5)
                break;
        }

        if (bestLag > 0) {
            double refinedLag = parabolicRefine(acf, bestLag);
            if (refinedLag > 0) {
                double acfFreq = m_sampleRate / refinedLag;
                if (method == Hybrid && freq > 0) {
                    /* 混合: 取YIN和ACF中置信度更高的 */
                    double acfConf = bestAcf;
                    if (acfConf > confidence) {
                        freq = acfFreq;
                        confidence = acfConf;
                    }
                } else {
                    freq = acfFreq;
                    confidence = bestAcf;
                }
            }
        }
    }

    result.frequency = freq;
    result.confidence = qBound(0.0, confidence, 1.0);
    result.voiced = (freq >= m_minFreq && freq <= m_maxFreq &&
                     confidence > m_threshold);

    if (result.voiced)
        m_freqHistory.append(freq);

    m_stats.totalFrames++;
    if (result.voiced) m_stats.totalVoiced++;
    else m_stats.totalUnvoiced++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalFrames;

    emit frameDetected(freq, confidence);
    return result;
}

QVector<PitchDetector::FrameResult> PitchDetector::detectBatch(
    const QVector<double>& signal, int frameSize, int hopSize)
{
    QVector<FrameResult> results;
    int pos = 0;
    while (pos + frameSize <= signal.size()) {
        QVector<double> frame(frameSize);
        std::copy(signal.begin() + pos, signal.begin() + pos + frameSize,
                  frame.begin());
        results.append(detect(frame));
        pos += hopSize;
    }
    return results;
}

QVector<QPair<double,int>> PitchDetector::frequencyHistogram(int bins) const
{
    if (m_freqHistory.isEmpty()) return {};

    double minF = *std::min_element(m_freqHistory.begin(), m_freqHistory.end());
    double maxF = *std::max_element(m_freqHistory.begin(), m_freqHistory.end());
    double range = maxF - minF;
    if (range < 1e-6) range = 1.0;

    QVector<QPair<double,int>> hist(bins);
    double binWidth = range / bins;
    for (int i = 0; i < bins; ++i)
        hist[i] = {minF + (i + 0.5) * binWidth, 0};

    for (double f : m_freqHistory) {
        int bin = static_cast<int>((f - minF) / binWidth);
        if (bin >= bins) bin = bins - 1;
        if (bin >= 0) hist[bin].second++;
    }

    return hist;
}

void PitchDetector::setVoicingThreshold(double threshold)
{
    m_threshold = qBound(0.0, threshold, 1.0);
}

QVector<double> PitchDetector::yinDifference(const QVector<double>& frame) const
{
    int n = frame.size() / 2;
    QVector<double> diff(n, 0.0);

    for (int tau = 0; tau < n; ++tau) {
        double sum = 0.0;
        for (int j = 0; j < n; ++j) {
            double d = frame[j] - frame[j + tau];
            sum += d * d;
        }
        diff[tau] = sum;
    }
    return diff;
}

QVector<double> PitchDetector::yinCMND(const QVector<double>& diff) const
{
    int n = diff.size();
    QVector<double> cmnd(n, 1.0);

    double runningSum = 0.0;
    for (int tau = 1; tau < n; ++tau) {
        runningSum += diff[tau];
        cmnd[tau] = (runningSum > 1e-12) ? diff[tau] * tau / runningSum : 1.0;
    }
    return cmnd;
}

QVector<double> PitchDetector::autocorrelation(const QVector<double>& frame) const
{
    int n = frame.size();
    QVector<double> acf(n, 0.0);

    for (int lag = 0; lag < n; ++lag) {
        double sum = 0.0;
        for (int i = 0; i < n - lag; ++i)
            sum += frame[i] * frame[i + lag];
        acf[lag] = sum;
    }

    /* 归一化 */
    if (acf[0] > 1e-12) {
        for (int i = 0; i < n; ++i)
            acf[i] /= acf[0];
    }
    return acf;
}

double PitchDetector::parabolicRefine(const QVector<double>& data,
                                       int idx) const
{
    if (idx <= 0 || idx >= data.size() - 1) return static_cast<double>(idx);

    double y0 = data[idx - 1];
    double y1 = data[idx];
    double y2 = data[idx + 1];
    double denom = 2.0 * (2.0 * y1 - y0 - y2);

    if (std::abs(denom) < 1e-15) return static_cast<double>(idx);

    double delta = (y0 - y2) / denom;
    return static_cast<double>(idx) + delta;
}

double PitchDetector::computeRMS(const QVector<double>& frame) const
{
    if (frame.isEmpty()) return 0.0;
    double sum = 0.0;
    for (double v : frame) sum += v * v;
    return std::sqrt(sum / frame.size());
}

void PitchDetector::resetStatistics()
{
    m_stats = Stats{};
    m_freqHistory.clear();
    m_timeSum = 0.0;
}
