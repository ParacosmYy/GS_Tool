/**
 * @file GoertzelAlgorithm2.cpp
 * @brief Goertzel算法2实现 — 多频并行检测+滑窗
 * @author EmbedDebug Team
 * @date 2026-06-05
 */

#include "utils/fft42/GoertzelAlgorithm2.h"

#include <QElapsedTimer>
#include <QtMath>

#include <algorithm>
#include <cmath>

/**
 * @brief 构造函数，初始化默认参数
 * @param parent 父对象
 */
GoertzelAlgorithm2::GoertzelAlgorithm2(QObject* parent)
    : QObject(parent)
{
    setObjectName(QStringLiteral("GoertzelAlgorithm2"));
}

/**
 * @brief 设置采样率
 * @param sampleRate 采样率（Hz）
 */
void GoertzelAlgorithm2::setSampleRate(double sampleRate)
{
    m_sampleRate = qMax(1.0, sampleRate);
}

/**
 * @brief 设置分析块大小
 *
 * 块大小N决定了频率分辨率为 sampleRate/N。
 *
 * @param blockSize 块大小（最小为8）
 */
void GoertzelAlgorithm2::setBlockSize(int blockSize)
{
    m_blockSize = qMax(8, blockSize);
}

/**
 * @brief 添加单个目标检测频率
 * @param freq 目标频率（Hz）
 */
void GoertzelAlgorithm2::addTargetFrequency(double freq)
{
    if (freq > 0 && freq < m_sampleRate / 2.0) {
        m_targetFreqs.append(freq);
    }
}

/**
 * @brief 批量添加目标检测频率
 * @param freqs 频率列表
 */
void GoertzelAlgorithm2::addTargetFrequencies(const QVector<double>& freqs)
{
    for (double f : freqs) {
        addTargetFrequency(f);
    }
}

/**
 * @brief 清除所有目标频率
 */
void GoertzelAlgorithm2::clearFrequencies()
{
    m_targetFreqs.clear();
    m_magnitudes.clear();
    m_phases.clear();
}

/**
 * @brief 计算所有目标频率的幅度和相位
 *
 * 使用Goertzel算法对每个目标频率独立计算DFT分量。
 * 相比完整FFT，在只需要少量频率时效率更高。
 * 滑窗处理：对长信号分块计算并取平均。
 *
 * @param signal 输入信号
 * @return 各目标频率的幅度值
 */
QVector<double> GoertzelAlgorithm2::compute(const QVector<double>& signal)
{
    QElapsedTimer timer;
    timer.start();

    if (signal.isEmpty() || m_targetFreqs.isEmpty()) {
        return {};
    }

    int numFreqs = m_targetFreqs.size();
    m_magnitudes.resize(numFreqs);
    m_phases.resize(numFreqs);

    /* 滑窗处理：将信号分成多个块 */
    int numBlocks = signal.size() / m_blockSize;
    if (numBlocks == 0) numBlocks = 1;

    for (int f = 0; f < numFreqs; ++f) {
        double magSum = 0.0;
        double phaseSum = 0.0;

        for (int block = 0; block < numBlocks; ++block) {
            int start = block * m_blockSize;
            int end = qMin(start + m_blockSize, signal.size());
            QVector<double> subSignal(end - start);
            for (int i = start; i < end; ++i) {
                subSignal[i - start] = signal[i];
            }

            auto result = singleGoertzel(subSignal, m_targetFreqs[f]);
            magSum += result.first;
            phaseSum += result.second;
        }

        m_magnitudes[f] = magSum / numBlocks;
        m_phases[f] = phaseSum / numBlocks;
    }

    /* 找峰值幅度 */
    double peakMag = 0.0;
    for (double m : m_magnitudes) {
        peakMag = qMax(peakMag, m);
    }

    m_stats.totalDetections++;
    m_stats.totalSamplesProcessed += signal.size();
    m_stats.numFrequencies = m_targetFreqs.size();

    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalDetections;

    emit detectionCompleted(m_targetFreqs.size(), peakMag);
    return m_magnitudes;
}

/**
 * @brief DTMF双音多频检测
 *
 * 使用标准DTMF频率组（低频组697/770/852/941Hz，
 * 高频组1209/1336/1477/1633Hz）进行检测。
 * 返回检测到的按键索引（0-15），-1表示未检测到。
 *
 * @param signal 输入信号
 * @return 按键索引（-1表示无有效DTMF）
 */
int GoertzelAlgorithm2::detectDTMF(const QVector<double>& signal) const
{
    if (signal.isEmpty()) return -1;

    /* 标准DTMF频率 */
    static const double lowFreqs[] = {697.0, 770.0, 852.0, 941.0};
    static const double highFreqs[] = {1209.0, 1336.0, 1477.0, 1633.0};

    double lowMag[4] = {};
    double highMag[4] = {};

    for (int i = 0; i < 4; ++i) {
        auto r = singleGoertzel(signal, lowFreqs[i]);
        lowMag[i] = r.first;
    }
    for (int i = 0; i < 4; ++i) {
        auto r = singleGoertzel(signal, highFreqs[i]);
        highMag[i] = r.first;
    }

    /* 找最大值索引 */
    int bestLow = 0, bestHigh = 0;
    for (int i = 1; i < 4; ++i) {
        if (lowMag[i] > lowMag[bestLow]) bestLow = i;
        if (highMag[i] > highMag[bestHigh]) bestHigh = i;
    }

    /* 检查有效性：最大值应明显大于次大值 */
    double secondLow = 0.0, secondHigh = 0.0;
    for (int i = 0; i < 4; ++i) {
        if (i != bestLow && lowMag[i] > secondLow) secondLow = lowMag[i];
        if (i != bestHigh && highMag[i] > secondHigh) secondHigh = highMag[i];
    }

    /* 信噪比阈值检测 */
    double threshold = 1.5;
    if (lowMag[bestLow] < secondLow * threshold ||
        highMag[bestHigh] < secondHigh * threshold) {
        return -1;
    }

    return bestLow * 4 + bestHigh;
}

/**
 * @brief 单频Goertzel算法
 *
 * 对指定目标频率计算DFT系数。使用递归二阶滤波器结构，
 * 避免完整FFT计算。复杂度O(N)，N为信号长度。
 *
 * @param signal 输入信号
 * @param targetFreq 目标频率
 * @return QPair<幅度, 相位>
 */
QPair<double,double> GoertzelAlgorithm2::singleGoertzel(
    const QVector<double>& signal, double targetFreq) const
{
    int N = signal.size();
    if (N == 0) return {0.0, 0.0};

    double k = (double)N * targetFreq / m_sampleRate;
    double w = 2.0 * M_PI * k / N;
    double coeff = 2.0 * qCos(w);

    double s0 = 0.0, s1 = 0.0, s2 = 0.0;

    for (int i = 0; i < N; ++i) {
        s0 = signal[i] + coeff * s1 - s2;
        s2 = s1;
        s1 = s0;
    }

    /* 计算复数DFT系数 */
    double realPart = s1 - s2 * qCos(w);
    double imagPart = s2 * qSin(w);

    double magnitude = qSqrt(realPart * realPart + imagPart * imagPart);
    double phase = qAtan2(imagPart, realPart);

    return {magnitude, phase};
}

/**
 * @brief 重置所有统计数据
 */
void GoertzelAlgorithm2::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
