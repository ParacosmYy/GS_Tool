/**
 * @file HarmonicProduct.cpp
 * @brief 谐波乘积谱实现 — 基频检测/自相关验证
 */

#include "utils/signal21/HarmonicProduct.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/** @brief 构造函数 @param parent 父对象 */
HarmonicProduct::HarmonicProduct(QObject* parent)
    : QObject(parent)
    , m_harmonicOrder(5)
    , m_minFreq(50.0)
    , m_maxFreq(2000.0)
{
}

/** @brief 设置谐波乘积阶数 @param order 阶数 */
void HarmonicProduct::setHarmonicOrder(int order)
{
    m_harmonicOrder = qMax(2, order);
}

/** @brief 设置频率搜索范围 @param minHz 最低频率 @param maxHz 最高频率 */
void HarmonicProduct::setSearchRange(double minHz, double maxHz)
{
    m_minFreq = qMax(1.0, minHz);
    m_maxFreq = qMax(m_minFreq + 1.0, maxHz);
}

/** @brief 计算谐波乘积谱 @param magnitude 幅度谱 @param frequencies 频率轴 @return HPS结果 */
QVector<double> HarmonicProduct::computeHPS(
    const QVector<double>& magnitude,
    const QVector<double>& frequencies)
{
    QElapsedTimer timer;
    timer.start();

    int n = magnitude.size();
    if (n < 2 || frequencies.size() != n) return {};

    /* HPS长度: 原始谱长度除以最大阶数 */
    int hpsLen = n / m_harmonicOrder;
    if (hpsLen < 1) return {};

    /* 初始化HPS为原始幅度谱(截断) */
    QVector<double> hps(hpsLen);
    for (int i = 0; i < hpsLen; ++i) {
        hps[i] = magnitude[i];
    }

    /* 逐阶降采样并相乘 */
    for (int order = 2; order <= m_harmonicOrder; ++order) {
        QVector<double> downsampled = downsampleSpectrum(magnitude, order);
        int minLen = qMin(hpsLen, downsampled.size());
        for (int i = 0; i < minLen; ++i) {
            hps[i] *= downsampled[i];
        }
    }

    /* 更新统计 */
    ++m_stats.totalSpectraProcessed;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(m_stats.totalDetections
                             + m_stats.totalSpectraProcessed);

    int peakBin = 0;
    double peakVal = 0.0;
    for (int i = 0; i < hpsLen; ++i) {
        if (hps[i] > peakVal) {
            peakVal = hps[i];
            peakBin = i;
        }
    }

    emit hpsComputed(m_harmonicOrder, peakBin);
    return hps;
}

/** @brief 检测基频 @param magnitude 幅度谱 @param frequencies 频率轴 @param sampleRate 采样率 */
HarmonicProduct::PitchResult HarmonicProduct::detectPitch(
    const QVector<double>& magnitude,
    const QVector<double>& frequencies,
    double sampleRate)
{
    QElapsedTimer timer;
    timer.start();

    PitchResult result;

    int n = magnitude.size();
    if (n < 2 || frequencies.size() != n) return result;

    /* 计算HPS */
    QVector<double> hps = computeHPS(magnitude, frequencies);
    if (hps.isEmpty()) return result;

    /* 确定搜索范围的bin索引 */
    double freqResolution = (n > 1) ? (frequencies[1] - frequencies[0]) : 1.0;
    int minBin = qMax(1, static_cast<int>(m_minFreq / qMax(freqResolution, 0.01)));
    int maxBin = qMin(hps.size() - 1,
                      static_cast<int>(m_maxFreq / qMax(freqResolution, 0.01)));

    if (minBin >= maxBin) return result;

    /* 在搜索范围内找HPS峰值 */
    int peakBin = findHpsPeak(hps, minBin, maxBin);
    if (peakBin < 0 || peakBin >= frequencies.size()) return result;

    result.frequency = frequencies[peakBin];
    result.hpsMagnitude = hps[peakBin];

    /* 计算置信度: 峰值与平均HPS的比值 */
    double avgHps = 0.0;
    for (int i = minBin; i <= maxBin; ++i) {
        avgHps += hps[i];
    }
    avgHps /= (maxBin - minBin + 1);
    result.confidence = (avgHps > 0) ? qMin(1.0, hps[peakBin] / (avgHps * 5.0)) : 0.0;

    /* 基频合理性验证 */
    result.valid = (result.frequency >= m_minFreq
                   && result.frequency <= m_maxFreq
                   && result.confidence > 0.1);

    /* 更新统计 */
    ++m_stats.totalDetections;
    if (result.valid) ++m_stats.totalValidPitches;
    double confSum = m_stats.avgConfidence * (m_stats.totalDetections - 1)
                   + result.confidence;
    m_stats.avgConfidence = confSum / m_stats.totalDetections;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(m_stats.totalDetections);

    emit pitchDetected(result.frequency, result.confidence);
    return result;
}

/** @brief 自相关验证 @param samples 时域采样 @param candidateFreq 候选基频 @param sampleRate 采样率 */
double HarmonicProduct::autocorrelationVerify(
    const QVector<double>& samples, double candidateFreq, double sampleRate)
{
    int n = samples.size();
    if (n == 0 || candidateFreq <= 0) return 0.0;

    /* 计算候选基频对应的周期(采样数) */
    double period = sampleRate / candidateFreq;
    int lag = qRound(period);
    if (lag <= 0 || lag >= n) return 0.0;

    /* 在候选周期及其倍数处计算自相关 */
    double corr1 = 0.0, corr2 = 0.0, corr3 = 0.0;
    double energy = 0.0;

    int maxLag = qMin(3 * lag, n - 1);
    int count = n - maxLag;

    for (int i = 0; i < count; ++i) {
        corr1 += samples[i] * samples[i + lag];
        if (2 * lag < n) corr2 += samples[i] * samples[i + 2 * lag];
        if (3 * lag < n) corr3 += samples[i] * samples[i + 3 * lag];
        energy += samples[i] * samples[i];
    }

    if (energy < 1e-20 || count <= 0) return 0.0;

    /* 归一化自相关 */
    corr1 /= (energy * count);
    corr2 /= (energy * count);
    corr3 /= (energy * count);

    /* 综合置信度: 基频和二次谐波应都有较高自相关 */
    double confidence = qMax(0.0, corr1) * 0.5
                      + qMax(0.0, corr2) * 0.3
                      + qMax(0.0, corr3) * 0.2;
    return qBound(0.0, confidence, 1.0);
}

/** @brief 降采样频谱 @param spectrum 频谱 @param factor 因子 @return 降采样结果 */
QVector<double> HarmonicProduct::downsampleSpectrum(
    const QVector<double>& spectrum, int factor)
{
    if (factor <= 1) return spectrum;

    int outLen = spectrum.size() / factor;
    QVector<double> result(outLen);

    for (int i = 0; i < outLen; ++i) {
        result[i] = spectrum[i * factor];
    }
    return result;
}

/** @brief 重置统计 */
void HarmonicProduct::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

/** @brief 在范围内找HPS峰值 @param hps HPS数据 @param minBin 最小bin @param maxBin 最大bin */
int HarmonicProduct::findHpsPeak(const QVector<double>& hps,
                                 int minBin, int maxBin)
{
    if (hps.isEmpty() || minBin < 0 || maxBin >= hps.size()) return -1;

    int peakBin = minBin;
    double peakVal = hps[minBin];

    for (int i = minBin + 1; i <= maxBin; ++i) {
        if (hps[i] > peakVal) {
            peakVal = hps[i];
            peakBin = i;
        }
    }
    return peakBin;
}
