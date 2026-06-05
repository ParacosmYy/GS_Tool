/**
 * @file HarmonicProduct3.cpp
 * @brief 谐波乘积谱基音检测实现
 *
 * 实现谐波乘积谱(HPS)算法用于基音频率检测:
 * 1. 对输入频谱进行逐级整数倍下采样
 * 2. 将原始频谱与各下采样频谱逐点相乘
 * 3. 乘积谱峰值位置对应基频FFT bin
 * 4. 通过抛物线插值获得亚bin精度
 *
 * @author EmbedDebug Team
 * @version 1.0
 * @date 2026-06-06
 */

#include "utils/signal78/HarmonicProduct3.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/**
 * @brief 构造函数，初始化HPS检测器默认参数
 * @param parent 父QObject指针
 */
HarmonicPitchDetector::HarmonicPitchDetector(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置采样率和HPS阶数
 * @param sampleRate 采样率(Hz)，必须大于0
 * @param harmonics 谐波级数(2~8)，默认5
 */
void HarmonicPitchDetector::initialize(double sampleRate, int harmonics)
{
    m_sampleRate = qMax(1.0, sampleRate);
    m_harmonics = qBound(2, harmonics, 8);
}

/**
 * @brief 从频谱幅度检测基音频率
 * @param magnitude FFT幅度谱(正频率部分)
 * @return 检测到的基音频率(Hz)，0表示未检测到
 */
double HarmonicPitchDetector::detectFromSpectrum(const QVector<double>& magnitude)
{
    QElapsedTimer timer;
    timer.start();

    double pitchHz = 0.0;

    const int specLen = magnitude.size();
    if (specLen < 4) {
        m_stats.totalFramesAnalyzed++;
        m_stats.totalPitchEstimates++;
        double elapsed = timer.elapsed();
        m_timeSum += elapsed;
        m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalPitchEstimates;
        emit pitchEstimated(0.0, 0.0);
        return 0.0;
    }

    /* 计算HPS可用最大bin数 */
    int maxBin = specLen;
    for (int h = 2; h <= m_harmonics; ++h) {
        maxBin = qMin(maxBin, specLen / h);
    }
    maxBin = qMax(1, maxBin);

    /* 初始化HPS为原频谱副本 */
    m_hps.resize(maxBin);
    for (int i = 0; i < maxBin; ++i) {
        m_hps[i] = qMax(magnitude[i], 1e-10);
    }

    /* 逐级下采样并相乘 */
    for (int h = 2; h <= m_harmonics; ++h) {
        for (int i = 0; i < maxBin; ++i) {
            int srcIdx = i * h;
            if (srcIdx < specLen) {
                m_hps[i] *= qMax(magnitude[srcIdx], 1e-10);
            }
        }
    }

    /* 搜索峰值(跳过直流bin=0) */
    int peakBin = 1;
    double peakVal = m_hps[1];
    for (int i = 2; i < maxBin; ++i) {
        if (m_hps[i] > peakVal) {
            peakVal = m_hps[i];
            peakBin = i;
        }
    }

    /* 抛物线插值精细化峰值位置 */
    double refinedBin = static_cast<double>(peakBin);
    if (peakBin > 0 && peakBin < maxBin - 1) {
        double alpha = m_hps[peakBin - 1];
        double beta = m_hps[peakBin];
        double gamma = m_hps[peakBin + 1];
        double denom = alpha - 2.0 * beta + gamma;
        if (qAbs(denom) > 1e-15) {
            double p = 0.5 * (alpha - gamma) / denom;
            p = qBound(-0.5, p, 0.5);
            refinedBin = peakBin + p;
        }
    }

    /* 转换为Hz频率 */
    double binResolution = m_sampleRate / (2.0 * specLen);
    pitchHz = qMax(0.0, refinedBin * binResolution * 2.0);

    /* 计算置信度 */
    double hpsSum = 0.0;
    for (int i = 1; i < maxBin; ++i) {
        hpsSum += m_hps[i];
    }
    double hpsMean = hpsSum / qMax(1, maxBin - 1);
    double confidence = (hpsMean > 1e-10) ? peakVal / (hpsMean * maxBin) : 0.0;
    confidence = qBound(0.0, confidence, 1.0);

    /* 更新统计 */
    m_stats.totalFramesAnalyzed++;
    m_stats.totalPitchEstimates++;
    double elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalPitchEstimates;

    emit pitchEstimated(pitchHz, confidence);
    return pitchHz;
}

/**
 * @brief 处理时域帧(内部先做FFT)
 * @param frame 时域音频帧
 * @return 检测到的基音频率(Hz)
 */
double HarmonicPitchDetector::detectFromFrame(const QVector<double>& frame)
{
    QElapsedTimer timer;
    timer.start();

    if (frame.isEmpty()) {
        m_stats.totalFramesAnalyzed++;
        m_stats.totalPitchEstimates++;
        double elapsed = timer.elapsed();
        m_timeSum += elapsed;
        m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalPitchEstimates;
        emit pitchEstimated(0.0, 0.0);
        return 0.0;
    }

    /* 计算DFT幅度谱(正频率部分) */
    const int N = frame.size();
    QVector<double> magnitude(N / 2 + 1, 0.0);

    for (int k = 0; k <= N / 2; ++k) {
        double re = 0.0;
        double im = 0.0;
        for (int n = 0; n < N; ++n) {
            double angle = -2.0 * M_PI * k * n / N;
            re += frame[n] * qCos(angle);
            im += frame[n] * qSin(angle);
        }
        magnitude[k] = qSqrt(re * re + im * im);
    }

    return detectFromSpectrum(magnitude);
}

/**
 * @brief 获取HPS谱
 * @return 最近一次计算的谐波乘积谱向量
 */
QVector<double> HarmonicPitchDetector::harmonicProductSpectrum() const
{
    return m_hps;
}

/**
 * @brief 重置所有统计数据
 */
void HarmonicPitchDetector::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_hps.clear();
}
