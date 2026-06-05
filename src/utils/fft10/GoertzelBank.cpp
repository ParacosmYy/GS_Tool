/**
 * @file GoertzelBank.cpp
 * @brief 多频率Goertzel滤波器组实现
 */

#include "GoertzelBank.h"
#include <QElapsedTimer>
#include <cmath>

GoertzelBank::GoertzelBank(QObject* parent)
    : QObject(parent)
    , m_timeSum(0.0)
    , m_sampleRate(0.0)
{
}

void GoertzelBank::setFrequencies(const QVector<double>& frequencies,
                                  double sampleRate)
{
    m_frequencies = frequencies;
    m_sampleRate = sampleRate;
}

QVector<GoertzelBank::ToneResult> GoertzelBank::detect(
    const QVector<double>& samples, double sampleRate, double thresholdDb)
{
    QElapsedTimer timer;
    timer.start();

    QVector<ToneResult> results;

    /* 使用配置的频率组,若为空则直接返回 */
    QVector<double> freqs = m_frequencies;
    if (freqs.isEmpty()) return results;

    for (double freq : freqs) {
        ToneResult tr = computeGoertzel(samples, freq, sampleRate, thresholdDb);
        results.append(tr);
    }

    /* 计算置信度: 各频率幅值相对最大幅值的比值 */
    double maxEnergy = 0.0;
    for (const auto& r : results) {
        if (r.energy > maxEnergy) maxEnergy = r.energy;
    }
    if (maxEnergy > 1e-15) {
        for (auto& r : results) {
            r.confidence = r.energy / maxEnergy;
        }
    }

    m_stats.totalDetections++;
    m_stats.totalTonesFound += std::count_if(results.begin(), results.end(),
        [](const ToneResult& t) { return t.detected; });
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalDetections;

    emit detectionCompleted(results.size());
    return results;
}

QPair<double, double> GoertzelBank::goertzelSingle(
    const QVector<double>& samples, double targetFreq, double sampleRate)
{
    int N = samples.size();
    if (N == 0) return {0.0, 0.0};

    double k = static_cast<double>(N) * targetFreq / sampleRate;
    double w = 2.0 * M_PI * k / N;
    double coeff = 2.0 * std::cos(w);

    double s0 = 0.0, s1 = 0.0, s2 = 0.0;
    for (int i = 0; i < N; ++i) {
        s0 = samples[i] + coeff * s1 - s2;
        s2 = s1;
        s1 = s0;
    }

    double real = s1 - s2 * std::cos(w);
    double imag = s2 * std::sin(w);
    double magnitude = std::sqrt(real * real + imag * imag);
    double phase = std::atan2(imag, real);

    return {magnitude, phase};
}

QString GoertzelBank::detectDTMF(const QVector<double>& samples,
                                 double sampleRate)
{
    QElapsedTimer timer;
    timer.start();

    /* DTMF标准频率: 行(低频组) + 列(高频组) */
    static const double lowFreqs[]  = {697.0, 770.0, 852.0, 941.0};
    static const double highFreqs[] = {1209.0, 1336.0, 1477.0, 1633.0};

    static const char* dtmfTable[4][4] = {
        {"1", "2", "3", "A"},
        {"4", "5", "6", "B"},
        {"7", "8", "9", "C"},
        {"*", "0", "#", "D"}
    };

    /* 检测所有8个频率的能量 */
    double lowEnergy[4] = {0}, highEnergy[4] = {0};
    double lowMag[4] = {0}, highMag[4] = {0};

    for (int i = 0; i < 4; ++i) {
        auto res = computeGoertzel(samples, lowFreqs[i], sampleRate, -40.0);
        lowEnergy[i] = res.energy;
        lowMag[i] = res.magnitude;
    }
    for (int i = 0; i < 4; ++i) {
        auto res = computeGoertzel(samples, highFreqs[i], sampleRate, -40.0);
        highEnergy[i] = res.energy;
        highMag[i] = res.magnitude;
    }

    /* 找最大能量的行和列 */
    int bestLow = 0, bestHigh = 0;
    for (int i = 1; i < 4; ++i) {
        if (lowEnergy[i] > lowEnergy[bestLow]) bestLow = i;
        if (highEnergy[i] > highEnergy[bestHigh]) bestHigh = i;
    }

    /* 能量阈值验证: 最强频率必须显著大于次强频率 */
    double maxLow = lowEnergy[bestLow];
    double secondLow = 0.0;
    for (int i = 0; i < 4; ++i) {
        if (i != bestLow && lowEnergy[i] > secondLow)
            secondLow = lowEnergy[i];
    }

    double maxHigh = highEnergy[bestHigh];
    double secondHigh = 0.0;
    for (int i = 0; i < 4; ++i) {
        if (i != bestHigh && highEnergy[i] > secondHigh)
            secondHigh = highEnergy[i];
    }

    /* 扭转检测: 高频/低频能量比须在合理范围 */
    QString result;
    double twistRatio = maxHigh / (maxLow + 1e-15);
    bool validTwist = (twistRatio > 0.2 && twistRatio < 5.0);
    bool dominantLow = (maxLow > secondLow * 2.0 + 1e-15);
    bool dominantHigh = (maxHigh > secondHigh * 2.0 + 1e-15);

    if (validTwist && dominantLow && dominantHigh &&
        maxLow > 1e-6 && maxHigh > 1e-6) {
        result = QString(dtmfTable[bestLow][bestHigh]);
    }

    m_stats.totalDetections++;
    m_stats.totalTonesFound += (result.isEmpty() ? 0 : 1);
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalDetections;

    return result;
}

QVector<double> GoertzelBank::configuredFrequencies() const
{
    return m_frequencies;
}

GoertzelBank::ToneResult GoertzelBank::computeGoertzel(
    const QVector<double>& samples, double targetFreq,
    double sampleRate, double thresholdDb) const
{
    ToneResult result;
    result.frequency = targetFreq;

    int N = samples.size();
    if (N == 0) return result;

    double k = static_cast<double>(N) * targetFreq / sampleRate;
    double w = 2.0 * M_PI * k / N;
    double coeff = 2.0 * std::cos(w);

    /* Goertzel核心迭代 */
    double s0 = 0.0, s1 = 0.0, s2 = 0.0;
    for (int i = 0; i < N; ++i) {
        s0 = samples[i] + coeff * s1 - s2;
        s2 = s1;
        s1 = s0;
    }

    /* 计算复数输出 */
    double real = s1 - s2 * std::cos(w);
    double imag = s2 * std::sin(w);
    result.magnitude = std::sqrt(real * real + imag * imag);
    result.phase = std::atan2(imag, real);
    result.energy = result.magnitude * result.magnitude;

    /* 计算信号总能量作为参考 */
    double totalEnergy = 0.0;
    for (int i = 0; i < N; ++i)
        totalEnergy += samples[i] * samples[i];

    /* 判断是否超过阈值 */
    if (totalEnergy > 1e-15) {
        double db = 10.0 * std::log10(result.energy / totalEnergy + 1e-30);
        result.detected = (db > thresholdDb);
    }

    return result;
}

GoertzelBank::Stats GoertzelBank::stats() const { return m_stats; }

void GoertzelBank::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
