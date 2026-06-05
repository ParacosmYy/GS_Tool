/**
 * @file NoiseProfiler.cpp
 * @brief NoiseProfiler 实现
 *
 * 实现噪声轮廓分析：频段划分、百分位噪声底估计、
 * 时间平滑和历史统计。
 */

#include "utils/signal164/NoiseProfiler.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/**
 * @brief 构造函数
 * @param parent 父QObject
 */
NoiseProfiler::NoiseProfiler(QObject* parent)
    : QObject(parent)
{
}

void NoiseProfiler::setSampleRate(double rate)
{
    m_sampleRate = qMax(8000.0, rate);
    m_initialized = false;
}

void NoiseProfiler::setFFTSize(int size)
{
    m_fftSize = qMax(64, size);
    m_initialized = false;
}

void NoiseProfiler::setPercentile(double percentile)
{
    m_percentile = qBound(0.0, percentile, 1.0);
}

void NoiseProfiler::setBandCount(int bands)
{
    m_bandCount = qMax(1, bands);
    m_initialized = false;
}

void NoiseProfiler::setSmoothing(double alpha)
{
    m_smoothing = qBound(0.0, alpha, 1.0);
}

/**
 * @brief 线性值转dB
 */
double NoiseProfiler::toDB(double linear)
{
    if (linear < 1e-20) return -200.0;
    return 20.0 * qLn(linear) / qLn(10.0);
}

/**
 * @brief 初始化频段划分
 *
 * 按对数等间距划分频率范围。
 */
void NoiseProfiler::initBands()
{
    m_bands.resize(m_bandCount);
    m_history.resize(m_bandCount);

    double nyquist = m_sampleRate / 2.0;
    int binsPerBand = m_fftSize / 2 / m_bandCount;

    for (int b = 0; b < m_bandCount; ++b) {
        int binStart = b * binsPerBand;
        int binEnd = (b + 1) * binsPerBand;
        if (b == m_bandCount - 1) binEnd = m_fftSize / 2;

        m_bands[b].lowFreq = binStart * m_sampleRate / m_fftSize;
        m_bands[b].highFreq = binEnd * m_sampleRate / m_fftSize;
        m_bands[b].binCount = binEnd - binStart;
        m_bands[b].noiseFloor = -100.0;
        m_bands[b].meanLevel = -100.0;
        m_bands[b].variance = 0.0;

        m_history[b].clear();
    }

    m_initialized = true;
}

/**
 * @brief 分析频谱，更新噪声轮廓
 *
 * 对每个频段：
 * 1) 收集该频段内所有频点的dB值
 * 2) 排序后取百分位数作为噪声底
 * 3) 用指数移动平均进行时间平滑
 */
QVector<NoiseProfiler::BandProfile> NoiseProfiler::profile(const QVector<double>& magnitude)
{
    QElapsedTimer timer;
    timer.start();

    if (!m_initialized) initBands();

    const int halfFFT = m_fftSize / 2;
    int binsPerBand = halfFFT / m_bandCount;

    double globalMin = 0.0;

    for (int b = 0; b < m_bandCount; ++b) {
        int binStart = b * binsPerBand;
        int binEnd = (b + 1) * binsPerBand;
        if (b == m_bandCount - 1) binEnd = qMin(halfFFT, magnitude.size());

        /* 收集dB值 */
        QVector<double> bandDB;
        bandDB.reserve(binEnd - binStart);
        for (int i = binStart; i < binEnd && i < magnitude.size(); ++i) {
            bandDB.append(toDB(magnitude[i]));
        }

        if (bandDB.isEmpty()) continue;

        /* 排序取百分位 */
        std::sort(bandDB.begin(), bandDB.end());
        int idx = static_cast<int>(m_percentile * (bandDB.size() - 1));
        double noiseFloor = bandDB[idx];

        /* 计算均值和方差 */
        double mean = 0.0;
        for (double v : bandDB) mean += v;
        mean /= bandDB.size();

        double variance = 0.0;
        for (double v : bandDB) {
            double diff = v - mean;
            variance += diff * diff;
        }
        variance /= bandDB.size();

        /* 时间平滑 */
        if (m_history[b].size() > 0) {
            double prevFloor = m_history[b].last();
            noiseFloor = m_smoothing * prevFloor + (1.0 - m_smoothing) * noiseFloor;
            mean = m_smoothing * m_bands[b].meanLevel + (1.0 - m_smoothing) * mean;
        }

        m_history[b].append(noiseFloor);
        m_bands[b].noiseFloor = noiseFloor;
        m_bands[b].meanLevel = mean;
        m_bands[b].variance = variance;

        if (b == 0 || noiseFloor < globalMin) {
            globalMin = noiseFloor;
        }
    }

    m_stats.globalNoiseFloor = globalMin;
    m_stats.bandCount = m_bandCount;
    m_stats.totalProfiles++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = (m_stats.totalProfiles > 0)
        ? m_timeSum / m_stats.totalProfiles : 0.0;

    emit profileCompleted(m_bandCount, globalMin);
    return m_bands;
}

void NoiseProfiler::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_initialized = false;
    for (auto& h : m_history) h.clear();
}
