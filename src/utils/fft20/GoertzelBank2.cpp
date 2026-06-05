/**
 * @file GoertzelBank2.cpp
 * @brief 多频率Goertzel滤波器组实现 — 定点频率检测/DTMF
 */

#include "utils/fft20/GoertzelBank2.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/** @brief 构造函数 @param parent 父对象 */
GoertzelBank2::GoertzelBank2(QObject* parent)
    : QObject(parent)
    , m_threshold(0.1)
{
    /* 初始化DTMF频率映射表: 行频率 x 列频率 → 字符 */
    double rowFreqs[] = {697.0, 770.0, 852.0, 941.0};
    double colFreqs[] = {1209.0, 1336.0, 1477.0, 1633.0};
    QChar dtmfChars[4][4] = {
        {'1', '2', '3', 'A'},
        {'4', '5', '6', 'B'},
        {'7', '8', '9', 'C'},
        {'*', '0', '#', 'D'}
    };

    for (int r = 0; r < 4; ++r) {
        for (int c = 0; c < 4; ++c) {
            m_dtmfMap[{rowFreqs[r], colFreqs[c]}] = dtmfChars[r][c];
        }
    }
}

/** @brief 设置目标频率列表 @param frequencies 频率列表 */
void GoertzelBank2::setTargetFrequencies(const QVector<double>& frequencies)
{
    m_targetFreqs = frequencies;
}

/** @brief 设置检测阈值(dB) @param threshold 阈值 */
void GoertzelBank2::setDetectionThreshold(double threshold)
{
    m_threshold = qPow(10.0, threshold / 20.0);
}

/** @brief 处理数据块 @param samples 采样数据 @param sampleRate 采样率 @return 检测结果 */
QList<GoertzelBank2::FrequencyResult> GoertzelBank2::processBank(
    const QVector<double>& samples, double sampleRate)
{
    QElapsedTimer timer;
    timer.start();

    QList<FrequencyResult> results;
    int n = samples.size();
    if (n == 0 || m_targetFreqs.isEmpty()) return results;

    /* 计算信号总能量作为参考 */
    double totalEnergy = computeEnergy(samples);

    for (double freq : m_targetFreqs) {
        FrequencyResult res;
        res.frequency = freq;

        auto pair = goertzelFilter(samples, freq, sampleRate);
        res.magnitude = pair.first;
        res.power = pair.second;
        res.detected = (res.magnitude > m_threshold * qSqrt(totalEnergy / n));

        results.append(res);
    }

    /* 更新统计 */
    double elapsed = timer.elapsed();
    ++m_stats.totalBlocksProcessed;
    m_stats.totalSamplesProcessed += n;
    for (const auto& r : results) {
        if (r.detected) ++m_stats.totalDetections;
    }
    double blockEnergy = totalEnergy / qMax(n, 1);
    double totalBlocks = static_cast<double>(m_stats.totalBlocksProcessed);
    m_stats.avgEnergyLevel = (m_stats.avgEnergyLevel * (totalBlocks - 1)
                             + blockEnergy) / totalBlocks;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / totalBlocks;

    emit frequenciesDetected(results);
    return results;
}

/** @brief 单Goertzel滤波器 @param samples 采样 @param targetFreq 目标频率 @param sampleRate 采样率 */
QPair<double, double> GoertzelBank2::goertzelFilter(
    const QVector<double>& samples, double targetFreq, double sampleRate)
{
    int n = samples.size();
    if (n == 0) return {0.0, 0.0};

    /* 计算Goertzel系数 */
    double coeff = computeCoefficient(targetFreq, sampleRate);
    double s0 = 0.0, s1 = 0.0, s2 = 0.0;

    /* Goertzel递推: s(n) = x(n) + 2*cos(w)*s(n-1) - s(n-2) */
    for (int i = 0; i < n; ++i) {
        s0 = samples[i] + coeff * s1 - s2;
        s2 = s1;
        s1 = s0;
    }

    /* 计算幅度和功率 */
    double real = s1 - s2 * qCos(2.0 * M_PI * targetFreq / sampleRate);
    double imag = s2 * qSin(2.0 * M_PI * targetFreq / sampleRate);
    double power = real * real + imag * imag;
    double magnitude = qSqrt(power) / n;

    return {magnitude, power};
}

/** @brief DTMF拨号音检测 @param samples 采样 @param sampleRate 采样率 @return DTMF结果 */
GoertzelBank2::DtmfResult GoertzelBank2::detectDTMF(
    const QVector<double>& samples, double sampleRate)
{
    QElapsedTimer timer;
    timer.start();

    DtmfResult result;
    result.confidence = 0.0;

    int n = samples.size();
    if (n == 0) return result;

    /* DTMF标准频率 */
    double rowFreqs[] = {697.0, 770.0, 852.0, 941.0};
    double colFreqs[] = {1209.0, 1336.0, 1477.0, 1633.0};

    /* 检测所有DTMF频率的幅度 */
    double rowMag[4], colMag[4];
    for (int i = 0; i < 4; ++i) {
        rowMag[i] = goertzelFilter(samples, rowFreqs[i], sampleRate).first;
        colMag[i] = goertzelFilter(samples, colFreqs[i], sampleRate).first;
    }

    /* 找到最大行和列频率 */
    int bestRow = 0, bestCol = 0;
    for (int i = 1; i < 4; ++i) {
        if (rowMag[i] > rowMag[bestRow]) bestRow = i;
        if (colMag[i] > colMag[bestCol]) bestCol = i;
    }

    /* 检测阈值验证 */
    double rowTotal = 0.0, colTotal = 0.0;
    for (int i = 0; i < 4; ++i) {
        rowTotal += rowMag[i];
        colTotal += colMag[i];
    }

    double rowConf = (rowTotal > 0) ? rowMag[bestRow] / rowTotal : 0.0;
    double colConf = (colTotal > 0) ? colMag[bestCol] / colTotal : 0.0;
    double confidence = rowConf * colConf;

    /* 查找DTMF字符 */
    result.rowFreq = rowFreqs[bestRow];
    result.colFreq = colFreqs[bestCol];
    result.confidence = confidence;

    auto it = m_dtmfMap.find({rowFreqs[bestRow], colFreqs[bestCol]});
    if (it != m_dtmfMap.end() && confidence > 0.4) {
        result.digit = it.value();
    } else {
        result.digit = QChar('?');
    }

    /* 更新统计 */
    double elapsed = timer.elapsed();
    ++m_stats.totalBlocksProcessed;
    m_stats.totalSamplesProcessed += n;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(m_stats.totalBlocksProcessed);

    emit dtmfDetected(result.digit, result.confidence);
    return result;
}

/** @brief 计算信号总能量 @param samples 采样数据 @return 能量值 */
double GoertzelBank2::computeEnergy(const QVector<double>& samples) const
{
    double energy = 0.0;
    for (double s : samples) {
        energy += s * s;
    }
    return energy;
}

/** @brief 重置统计 */
void GoertzelBank2::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

/** @brief 计算Goertzel系数 @param targetFreq 目标频率 @param sampleRate 采样率 @return 系数 */
double GoertzelBank2::computeCoefficient(double targetFreq,
                                         double sampleRate) const
{
    return 2.0 * qCos(2.0 * M_PI * targetFreq / sampleRate);
}
