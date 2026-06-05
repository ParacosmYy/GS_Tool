/**
 * @file NoiseProfile2.cpp
 * @brief 噪声轮廓2实现 — 自适应噪声估计+谱掩码
 *
 * 实现自适应噪声轮廓估计器，通过指数移动平均在线更新噪声估计，
 * 支持噪声/信号帧标记，生成谱掩码用于降噪处理。
 */

#include "utils/signal47/NoiseProfile2.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/**
 * @brief 构造函数，初始化默认参数
 * @param parent 父QObject
 */
NoiseProfile2::NoiseProfile2(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置采样率
 * @param sampleRate 采样率(Hz)
 */
void NoiseProfile2::setSampleRate(double sampleRate)
{
    m_sampleRate = qMax(1.0, sampleRate);
}

/**
 * @brief 设置FFT大小
 * @param fftSize FFT窗口大小
 */
void NoiseProfile2::setFFTSize(int fftSize)
{
    m_fftSize = qMax(64, fftSize);
    m_noiseEstimate.clear();
    m_noisePower.clear();
    m_signalPower.clear();
    m_framesLearned = 0;
}

/**
 * @brief 设置自适应更新速率
 * @param rate 速率因子[0,1]，越大表示新数据权重越高
 */
void NoiseProfile2::setAdaptationRate(double rate)
{
    m_adaptRate = qBound(0.0, rate, 1.0);
}

/**
 * @brief 用新的频谱帧更新噪声轮廓
 * @param spectrum 输入频谱幅度（FFT后的幅度谱）
 * @param isNoise true标记为噪声帧，false标记为信号帧
 *
 * 噪声帧用于直接更新噪声估计；信号帧仅更新信号功率统计。
 * 使用指数移动平均实现自适应跟踪。
 */
void NoiseProfile2::update(const QVector<double>& spectrum, bool isNoise)
{
    QElapsedTimer timer;
    timer.start();

    const int n = spectrum.size();
    if (n == 0) return;

    /* 初始化估计数组 */
    if (m_noiseEstimate.size() != n) {
        m_noiseEstimate = spectrum;
        m_noisePower.resize(n, 0.0);
        m_signalPower.resize(n, 0.0);
        for (int i = 0; i < n; ++i) {
            m_noisePower[i] = spectrum[i] * spectrum[i];
        }
    }

    const double alpha = m_adaptRate;
    const double oneMinusAlpha = 1.0 - alpha;

    if (isNoise) {
        /* 噪声帧: 指数移动平均更新噪声估计 */
        for (int i = 0; i < n; ++i) {
            double power = spectrum[i] * spectrum[i];
            m_noisePower[i] = oneMinusAlpha * m_noisePower[i] + alpha * power;
            m_noiseEstimate[i] = qSqrt(m_noisePower[i]);
        }
    } else {
        /* 信号帧: 更新信号功率统计 */
        for (int i = 0; i < n; ++i) {
            double power = spectrum[i] * spectrum[i];
            m_signalPower[i] = oneMinusAlpha * m_signalPower[i] + alpha * power;
        }

        /* 自适应: 如果信号功率接近噪声功率，可能噪声发生了变化 */
        for (int i = 0; i < n; ++i) {
            double sigEst = qSqrt(m_signalPower[i]);
            double noiseEst = m_noiseEstimate[i];
            /* 当信号功率很低时，缓慢更新噪声估计 */
            double ratio = (noiseEst > 1e-12) ? sigEst / noiseEst : 1.0;
            if (ratio < 1.2) {
                double slowAlpha = alpha * 0.1;
                m_noisePower[i] = (1.0 - slowAlpha) * m_noisePower[i]
                                   + slowAlpha * spectrum[i] * spectrum[i];
                m_noiseEstimate[i] = qSqrt(m_noisePower[i]);
            }
        }
    }

    m_framesLearned++;

    /* 更新统计 */
    double elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.totalUpdates++;
    m_stats.totalFramesAnalyzed++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalUpdates;

    double noiseFloor = 0.0;
    for (int i = 0; i < m_noiseEstimate.size(); ++i) {
        noiseFloor += m_noiseEstimate[i];
    }
    noiseFloor /= m_noiseEstimate.size();

    emit profileUpdated(m_framesLearned, noiseFloor);
}

/**
 * @brief 生成谱掩码（Wiener增益）
 * @return 增益掩码向量[0,1]，0表示完全抑制，1表示完全保留
 *
 * 使用维纳滤波增益公式: G = max(1 - noise/signal, floor)
 */
QVector<double> NoiseProfile2::noiseMask() const
{
    const int n = m_noiseEstimate.size();
    QVector<double> mask(n, 1.0);

    for (int i = 0; i < n; ++i) {
        double signalMag = qSqrt(m_signalPower[i]);
        double noiseMag = m_noiseEstimate[i];

        if (signalMag > 1e-12 && noiseMag > 1e-12) {
            /* 维纳增益: G = 1 - (noise/signal) */
            double gain = 1.0 - noiseMag / signalMag;
            mask[i] = qBound(0.01, gain, 1.0);
        } else if (noiseMag > 1e-12) {
            mask[i] = 0.01;
        }
    }

    return mask;
}

/**
 * @brief 计算当前信噪比估计
 * @return 平均信噪比(dB)
 */
double NoiseProfile2::snr() const
{
    const int n = m_noiseEstimate.size();
    if (n == 0) return 0.0;

    double totalSignal = 0.0;
    double totalNoise = 0.0;

    for (int i = 0; i < n; ++i) {
        totalSignal += m_signalPower[i];
        totalNoise += m_noisePower[i];
    }

    if (totalNoise < 1e-24) return 60.0;
    double snrLin = totalSignal / totalNoise;
    return 10.0 * qLn(qMax(1e-12, snrLin)) / qLn(10.0);
}

/**
 * @brief 重置所有统计信息
 */
void NoiseProfile2::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

/**
 * @brief 重置噪声估计和所有学习到的数据
 *
 * 清除噪声功率、信号功率估计和已学习帧计数，
 * 使估计器恢复到初始状态，准备接受新的噪声轮廓学习。
 */
void NoiseProfile2::resetProfile()
{
    m_noiseEstimate.clear();
    m_noisePower.clear();
    m_signalPower.clear();
    m_framesLearned = 0;
}

/**
 * @brief 获取噪声底噪估计(dB)
 * @return 噪声底噪(dB)，基于噪声功率均值计算
 *
 * 噪声底噪 = 10 * log10(mean(noisePower))
 * 用于评估整体噪声水平。
 */
double NoiseProfile2::noiseFloorDb() const
{
    const int n = m_noisePower.size();
    if (n == 0) return -96.0;

    double totalPower = 0.0;
    for (int i = 0; i < n; ++i) {
        totalPower += m_noisePower[i];
    }
    double avgPower = totalPower / n;
    return 10.0 * (qLn(qMax(1e-12, avgPower)) / qLn(10.0));
}

/**
 * @brief 获取频带信噪比向量
 * @return 每个频率bin的信噪比(dB)数组
 *
 * 逐bin计算 SNR = 10 * log10(signalPower / noisePower)，
 * 用于分析不同频段的噪声情况。
 */
QVector<double> NoiseProfile2::bandSnr() const
{
    const int n = m_noisePower.size();
    QVector<double> bandSnrVec(n, 0.0);

    for (int i = 0; i < n; ++i) {
        if (m_noisePower[i] > 1e-24) {
            double ratio = m_signalPower[i] / m_noisePower[i];
            bandSnrVec[i] = 10.0 * (qLn(qMax(1e-12, ratio)) / qLn(10.0));
        } else {
            bandSnrVec[i] = 60.0; /* 噪声极低，SNR很高 */
        }
    }

    return bandSnrVec;
}

/**
 * @brief 获取指定频段的平均信噪比
 * @param lowBin 起始频率bin索引
 * @param highBin 结束频率bin索引
 * @return 该频段的平均SNR(dB)
 */
double NoiseProfile2::bandSnrRange(int lowBin, int highBin) const
{
    const int n = m_noisePower.size();
    lowBin = qBound(0, lowBin, n - 1);
    highBin = qBound(lowBin, highBin, n - 1);

    double totalSignal = 0.0;
    double totalNoise = 0.0;
    int count = highBin - lowBin + 1;

    for (int i = lowBin; i <= highBin; ++i) {
        totalSignal += m_signalPower[i];
        totalNoise += m_noisePower[i];
    }

    if (totalNoise < 1e-24 || count == 0) return 60.0;
    double ratio = totalSignal / totalNoise;
    return 10.0 * (qLn(qMax(1e-12, ratio)) / qLn(10.0));
}
