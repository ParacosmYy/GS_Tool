/**
 * @file GoertzelAlgorithm.cpp
 * @brief Goertzel算法实现 — 单频率DFT/频点检测/DTMF解码
 * @author EmbedDebug Team
 * @date 2026-06-05
 */

#include "utils/fft30/GoertzelAlgorithm.h"

#include <QElapsedTimer>
#include <QtMath>

#include <algorithm>

/** @brief 构造函数 @param parent 父对象 */
GoertzelAlgorithm::GoertzelAlgorithm(QObject* parent)
    : QObject(parent)
{
}

/** @brief 设置采样率 @param rate 采样率(Hz) */
void GoertzelAlgorithm::setSampleRate(double rate)
{
    m_sampleRate = qMax(1.0, rate);
}

/** @brief 添加目标检测频率 @param freq 目标频率(Hz) */
void GoertzelAlgorithm::addTargetFrequency(double freq)
{
    if (freq > 0.0 && !m_targetFreqs.contains(freq)) {
        m_targetFreqs.append(freq);
    }
}

/** @brief 批量添加目标检测频率 @param freqs 目标频率列表 */
void GoertzelAlgorithm::addTargetFrequencies(const QVector<double>& freqs)
{
    for (double f : freqs) {
        addTargetFrequency(f);
    }
}

/** @brief 清除所有目标频率 */
void GoertzelAlgorithm::clearTargets()
{
    m_targetFreqs.clear();
}

/** @brief 对指定频率执行Goertzel算法
 *  @param samples 输入采样数据
 *  @param targetFreq 目标频率(Hz)
 *  @return 频率分析结果(幅度/相位/功率)
 */
GoertzelAlgorithm::FreqResult GoertzelAlgorithm::compute(
    const QVector<double>& samples, double targetFreq) const
{
    FreqResult result;
    result.frequency = targetFreq;

    int N = samples.size();
    if (N < 2 || targetFreq <= 0.0 || m_sampleRate <= 0.0) {
        return result;
    }

    /* 计算Goertzel系数 */
    double k = 0.5 + N * targetFreq / m_sampleRate;
    double w = 2.0 * M_PI * k / N;
    double coeff = 2.0 * qCos(w);

    /* 递推计算 */
    double s0 = 0.0, s1 = 0.0, s2 = 0.0;
    for (int i = 0; i < N; ++i) {
        s0 = samples[i] + coeff * s1 - s2;
        s2 = s1;
        s1 = s0;
    }

    /* 计算复数结果 */
    double real = s1 - s2 * qCos(w);
    double imag = s2 * qSin(w);

    result.magnitude = qSqrt(real * real + imag * imag);
    result.phase = qAtan2(imag, real);
    result.power = real * real + imag * imag;

    return result;
}

/** @brief 对所有目标频率执行Goertzel算法
 *  @param samples 输入采样数据
 *  @return 各频率分析结果列表
 */
QVector<GoertzelAlgorithm::FreqResult> GoertzelAlgorithm::computeAll(
    const QVector<double>& samples)
{
    QElapsedTimer timer;
    timer.start();

    QVector<FreqResult> results;
    results.reserve(m_targetFreqs.size());

    for (double freq : m_targetFreqs) {
        results.append(compute(samples, freq));
    }

    m_stats.totalDetections += m_targetFreqs.size();
    m_stats.totalSamplesProcessed += samples.size();
    qint64 elapsed = timer.elapsed();
    m_timeSum += static_cast<double>(elapsed);
    m_stats.avgProcessingTimeMs = (m_stats.totalDetections > 0) ?
        m_timeSum / m_stats.totalDetections : 0.0;
    emit detectionComplete(m_targetFreqs.size());
    return results;
}

/** @brief DTMF双音多频检测
 *
 *  检测标准DTMF频率对:
 *  低频组: 697, 770, 852, 941 Hz
 *  高频组: 1209, 1336, 1477, 1633 Hz
 *
 *  @param samples 输入采样数据
 *  @return 检测到的DTMF音列表(字符, 置信度)
 */
QVector<QPair<double,double>> GoertzelAlgorithm::dtmfDetect(
    const QVector<double>& samples)
{
    QElapsedTimer timer;
    timer.start();

    QVector<QPair<double,double>> results;

    /* DTMF标准频率 */
    static const double lowFreqs[] = {697.0, 770.0, 852.0, 941.0};
    static const double highFreqs[] = {1209.0, 1336.0, 1477.0, 1633.0};

    /* DTMF字符映射 (行 x 列) */
    static const char dtmfMap[4][4] = {
        {'1', '2', '3', 'A'},
        {'4', '5', '6', 'B'},
        {'7', '8', '9', 'C'},
        {'*', '0', '#', 'D'}
    };

    /* 计算低频组功率 */
    double lowPower[4] = {0.0};
    int bestLow = 0;
    for (int i = 0; i < 4; ++i) {
        FreqResult r = compute(samples, lowFreqs[i]);
        lowPower[i] = r.power;
        if (lowPower[i] > lowPower[bestLow]) {
            bestLow = i;
        }
    }

    /* 计算高频组功率 */
    double highPower[4] = {0.0};
    int bestHigh = 0;
    for (int i = 0; i < 4; ++i) {
        FreqResult r = compute(samples, highFreqs[i]);
        highPower[i] = r.power;
        if (highPower[i] > highPower[bestHigh]) {
            bestHigh = i;
        }
    }

    /* 判决: 检查是否存在有效的DTMF信号 */
    double lowSum = 0.0, highSum = 0.0;
    for (int i = 0; i < 4; ++i) {
        lowSum += lowPower[i];
        highSum += highPower[i];
    }

    /* 功率阈值检测: 最强频率功率应占总功率的显著比例 */
    double lowRatio = (lowSum > 1e-12) ? lowPower[bestLow] / lowSum : 0.0;
    double highRatio = (highSum > 1e-12) ? highPower[bestHigh] / highSum : 0.0;

    double threshold = 0.4;
    if (lowRatio > threshold && highRatio > threshold) {
        char tone = dtmfMap[bestLow][bestHigh];
        double confidence = qMin(lowRatio, highRatio);
        results.append(qMakePair(static_cast<double>(tone), confidence));
        emit dtmfToneDetected(tone, confidence);
    }

    m_stats.totalDetections++;
    m_stats.totalSamplesProcessed += samples.size();
    qint64 elapsed = timer.elapsed();
    m_timeSum += static_cast<double>(elapsed);
    m_stats.avgProcessingTimeMs = (m_stats.totalDetections > 0) ?
        m_timeSum / m_stats.totalDetections : 0.0;
    return results;
}

/** @brief 重置所有统计计数器 */
void GoertzelAlgorithm::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
