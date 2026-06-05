/**
 * @file BeatDetector.cpp
 * @brief 节拍检测实现 — onset函数/自相关/节奏直方图/Tempo估计
 */

#include "utils/signal30/BeatDetector.h"

#include <QtMath>
#include <algorithm>
#include <numeric>

BeatDetector::BeatDetector(double sampleRate, int hopSize, int fftSize, QObject* parent)
    : QObject(parent), m_sampleRate(sampleRate), m_hopSize(hopSize),
      m_fftSize(fftSize), m_onsetThreshold(0.3), m_minBPM(60.0), m_maxBPM(200.0)
{
}

BeatDetector::BeatResult BeatDetector::detect(const QVector<float>& samples)
{
    m_timing.start();
    ++m_stats.totalTempoEstimates;

    BeatResult result;
    result.estimatedTempoBPM = 120.0;
    result.tempoConfidence = 0.0;

    /* 1. 计算onset检测函数 */
    QVector<double> onsetFunc = computeOnsetFunction(samples);
    int nFrames = onsetFunc.size();
    m_stats.totalFramesProcessed += nFrames;

    if (nFrames < 4) {
        m_timeSum += m_timing.elapsed();
        m_stats.avgProcessingTimeMs = (m_stats.totalTempoEstimates > 0)
            ? m_timeSum / m_stats.totalTempoEstimates : 0.0;
        return result;
    }

    /* 2. onset峰值拾取 */
    QVector<int> peaks = pickPeaks(onsetFunc, m_onsetThreshold);
    double frameDur = static_cast<double>(m_hopSize) / m_sampleRate;
    for (int p : peaks) {
        result.onsetTimes.append(p * frameDur);
        result.onsetStrength.append(onsetFunc[p]);
        emit onsetDetected(p * frameDur, onsetFunc[p]);
    }
    m_stats.totalOnsetsDetected += peaks.size();

    /* 3. Tempo估计 */
    result.estimatedTempoBPM = estimateTempo(onsetFunc);

    /* 4. 节奏直方图 */
    result.tempoHistogram = buildTempoHistogram(onsetFunc);

    /* 5. 置信度: 直方图峰值归一化 */
    double maxBin = 0.0, totalBin = 0.0;
    for (double v : result.tempoHistogram) {
        maxBin = qMax(maxBin, v);
        totalBin += v;
    }
    result.tempoConfidence = (totalBin > 0.0) ? maxBin / totalBin : 0.0;

    m_timeSum += m_timing.elapsed();
    m_stats.avgProcessingTimeMs = (m_stats.totalTempoEstimates > 0)
        ? m_timeSum / m_stats.totalTempoEstimates : 0.0;
    emit detectionComplete(peaks.size(), result.estimatedTempoBPM,
                           m_stats.avgProcessingTimeMs);
    return result;
}

QVector<double> BeatDetector::computeOnsetFunction(const QVector<float>& samples)
{
    int nFrames = (samples.size() - m_fftSize) / m_hopSize + 1;
    if (nFrames <= 0) return {};

    QVector<double> onsetFunc(nFrames, 0.0);
    QVector<double> prevMag(m_fftSize / 2, 0.0);
    QVector<double> currMag(m_fftSize / 2);

    for (int f = 0; f < nFrames; ++f) {
        /* 提取帧 */
        QVector<float> frame(m_fftSize, 0.0f);
        int start = f * m_hopSize;
        for (int i = 0; i < m_fftSize && start + i < samples.size(); ++i)
            frame[i] = samples[start + i];
        /* 汉宁窗 */
        for (int i = 0; i < m_fftSize; ++i)
            frame[i] *= 0.5f * (1.0f - qCos(2.0f * static_cast<float>(M_PI) * i / m_fftSize));

        computeMagnitudeSpectrum(frame, currMag);
        onsetFunc[f] = spectralFlux(prevMag, currMag);
        prevMag = currMag;
    }
    return onsetFunc;
}

double BeatDetector::estimateTempo(const QVector<double>& onsetFunction)
{
    int n = onsetFunction.size();
    if (n < 4) return 120.0;
    /* 自相关 */
    double frameDur = static_cast<double>(m_hopSize) / m_sampleRate;
    int minLag = static_cast<int>(60.0 / (m_maxBPM * frameDur));
    int maxLag = static_cast<int>(60.0 / (m_minBPM * frameDur));
    minLag = qMax(1, minLag);
    maxLag = qMin(n - 1, maxLag);
    QVector<double> acf = autocorrelation(onsetFunction, maxLag);

    /* 找自相关峰值对应的lag */
    double bestVal = -1e30;
    int bestLag = minLag;
    for (int lag = minLag; lag <= maxLag; ++lag) {
        if (acf[lag] > bestVal) {
            bestVal = acf[lag];
            bestLag = lag;
        }
    }
    return 60.0 / (bestLag * frameDur);
}

void BeatDetector::computeMagnitudeSpectrum(const QVector<float>& frame,
                                             QVector<double>& mag) const
{
    int n = frame.size();
    QVector<double> real(n), imag(n, 0.0);
    for (int i = 0; i < n; ++i) real[i] = frame[i];
    /* 基2 DFT(简化实现) */
    for (int k = 0; k < n / 2; ++k) {
        double re = 0.0, im = 0.0;
        for (int i = 0; i < n; ++i) {
            double angle = -2.0 * M_PI * k * i / n;
            re += real[i] * qCos(angle);
            im += real[i] * qSin(angle);
        }
        mag[k] = qSqrt(re * re + im * im);
    }
}

double BeatDetector::spectralFlux(const QVector<double>& prev,
                                   const QVector<double>& curr) const
{
    double flux = 0.0;
    int n = qMin(prev.size(), curr.size());
    for (int i = 0; i < n; ++i) {
        double diff = curr[i] - prev[i];
        flux += qMax(0.0, diff); /* 半波整流 */
    }
    return flux;
}

QVector<int> BeatDetector::pickPeaks(const QVector<double>& func, double threshold) const
{
    QVector<int> peaks;
    int n = func.size();
    if (n < 3) return peaks;
    /* 计算局部均值和标准差用于自适应阈值 */
    double mean = 0.0;
    for (double v : func) mean += v;
    mean /= n;
    double stddev = 0.0;
    for (double v : func) stddev += (v - mean) * (v - mean);
    stddev = qSqrt(stddev / n);
    double thresh = mean + threshold * stddev;

    for (int i = 1; i < n - 1; ++i) {
        if (func[i] > thresh && func[i] > func[i - 1] && func[i] >= func[i + 1]) {
            if (peaks.isEmpty() || i - peaks.last() > 3)
                peaks.append(i);
        }
    }
    return peaks;
}

QVector<double> BeatDetector::autocorrelation(const QVector<double>& signal,
                                               int maxLag) const
{
    QVector<double> acf(maxLag + 1, 0.0);
    int n = signal.size();
    for (int lag = 0; lag <= maxLag; ++lag) {
        double sum = 0.0;
        for (int i = 0; i < n - lag; ++i)
            sum += signal[i] * signal[i + lag];
        acf[lag] = sum;
    }
    /* 归一化 */
    if (acf[0] > 0.0) {
        for (double& v : acf) v /= acf[0];
    }
    return acf;
}

QVector<double> BeatDetector::buildTempoHistogram(const QVector<double>& onsetFunc) const
{
    double frameDur = static_cast<double>(m_hopSize) / m_sampleRate;
    int nBins = 150;
    QVector<double> histogram(nBins, 0.0);

    /* 计算所有onset帧间的IOI */
    QVector<int> peaks = pickPeaks(onsetFunc, m_onsetThreshold);
    for (int i = 0; i < peaks.size(); ++i) {
        for (int j = i + 1; j < peaks.size(); ++j) {
            double ioiSec = (peaks[j] - peaks[i]) * frameDur;
            double bpm = 60.0 / ioiSec;
            if (bpm >= m_minBPM && bpm <= m_maxBPM) {
                int bin = static_cast<int>((bpm - m_minBPM) / (m_maxBPM - m_minBPM) * nBins);
                bin = qBound(0, bin, nBins - 1);
                histogram[bin] += 1.0 / (j - i); /* 近距离IOI权重高 */
            }
        }
    }
    return histogram;
}

void BeatDetector::setTempoRange(double minBPM, double maxBPM)
{
    m_minBPM = qBound(30.0, minBPM, 300.0);
    m_maxBPM = qBound(m_minBPM + 10.0, maxBPM, 400.0);
}

void BeatDetector::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
