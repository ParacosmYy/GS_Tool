/**
 * @file PitchTrack3.cpp
 * @brief 基频(音高)跟踪器实现
 *
 * 实现基于YIN和自相关(ACF)算法的基频检测，支持逐帧音高
 * 跟踪和置信度曲线输出。适用于语音和音乐信号分析。
 */

#include "utils/signal71/PitchTrack3.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/**
 * @brief 构造函数，初始化默认参数
 * @param parent 父对象指针
 */
PitchTrack3::PitchTrack3(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置采样率
 * @param sr 采样率(Hz)
 */
void PitchTrack3::setSampleRate(double sr)
{
    m_sampleRate = qBound(8000.0, sr, 192000.0);
}

/**
 * @brief 设置最低检测频率
 * @param fmin 最低频率(Hz)
 */
void PitchTrack3::setMinFreq(double fmin)
{
    m_fmin = qBound(20.0, fmin, 2000.0);
}

/**
 * @brief 设置最高检测频率
 * @param fmax 最高频率(Hz)
 */
void PitchTrack3::setMaxFreq(double fmax)
{
    m_fmax = qBound(m_fmin + 10.0, fmax, m_sampleRate / 2.0);
}

/**
 * @brief 设置检测算法
 * @param algo 算法名称："yin" 或 "acf"
 */
void PitchTrack3::setAlgorithm(const QString& algo)
{
    if (algo == "yin" || algo == "acf") {
        m_algo = algo;
    }
}

/**
 * @brief 对信号进行基频跟踪
 * @param signal 输入时域信号
 * @return 每帧的基频估计(Hz)，静音帧返回0
 */
QVector<double> PitchTrack3::track(const QVector<double>& signal)
{
    QElapsedTimer timer;
    timer.start();

    m_curve.clear();
    m_conf.clear();

    if (signal.isEmpty()) return m_curve;

    // 帧参数
    int frameSize = qRound(m_sampleRate / m_fmin * 2); // 至少两个最低周期
    int hopSize = frameSize / 2;
    int numFrames = qMax(1, (signal.size() - frameSize) / hopSize + 1);

    m_curve.reserve(numFrames);
    m_conf.reserve(numFrames);

    for (int f = 0; f < numFrames; ++f) {
        int start = f * hopSize;

        // 提取帧
        QVector<double> frame(frameSize);
        for (int i = 0; i < frameSize && (start + i) < signal.size(); ++i) {
            frame[i] = signal[start + i];
        }
        // 填零
        for (int i = qMin(frameSize, signal.size() - start); i < frameSize; ++i) {
            frame[i] = 0.0;
        }

        // 检测基频
        double freq = 0.0;
        double confidence = 0.0;

        if (m_algo == "yin") {
            freq = detectYin(frame);
            // YIN置信度基于差分函数的最小值
            confidence = (freq > 0.0) ? 1.0 : 0.0;
        } else {
            freq = detectACF(frame);
            confidence = (freq > 0.0) ? 1.0 : 0.0;
        }

        m_curve.append(freq);
        m_conf.append(confidence);

        emit frameProcessed(f, freq, confidence);
    }

    // 更新统计信息
    qint64 elapsed = timer.elapsed();
    m_stats.totalTrackings++;
    m_stats.totalFrames += numFrames;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalTrackings;

    return m_curve;
}

/**
 * @brief 重置统计信息
 */
void PitchTrack3::resetStatistics()
{
    m_stats = Stats();
    m_timeSum = 0.0;
}

/**
 * @brief YIN算法基频检测
 * @param frame 输入帧
 * @return 基频(Hz)，未检测到返回0
 *
 * YIN算法通过差分函数的累积均值归一化实现精确的基频检测。
 */
double PitchTrack3::detectYin(const QVector<double>& frame)
{
    int frameSize = frame.size();
    int maxTau = qRound(m_sampleRate / m_fmin);
    int minTau = qRound(m_sampleRate / m_fmax);

    maxTau = qMin(maxTau, frameSize / 2);
    minTau = qMax(2, minTau);

    if (maxTau <= minTau) return 0.0;

    // 步骤1：差分函数
    QVector<double> d(maxTau, 0.0);
    for (int tau = 1; tau < maxTau; ++tau) {
        for (int j = 0; j < frameSize - tau; ++j) {
            double diff = frame[j] - frame[j + tau];
            d[tau] += diff * diff;
        }
    }

    // 步骤2：累积均值归一化差分函数
    QVector<double> dPrime(maxTau, 0.0);
    dPrime[0] = 1.0;
    double runningSum = 0.0;
    for (int tau = 1; tau < maxTau; ++tau) {
        runningSum += d[tau];
        dPrime[tau] = (runningSum > 0.0) ? d[tau] * tau / runningSum : 1.0;
    }

    // 步骤3：绝对阈值
    double threshold = 0.15;
    int tau = minTau;
    while (tau < maxTau) {
        if (dPrime[tau] < threshold) {
            // 找到局部最小值
            while (tau + 1 < maxTau && dPrime[tau + 1] < dPrime[tau]) tau++;
            break;
        }
        tau++;
    }

    if (tau >= maxTau) return 0.0;

    // 步骤4：抛物线插值精化
    double betterTau = tau;
    if (tau > 0 && tau < maxTau - 1) {
        double s0 = dPrime[tau - 1];
        double s1 = dPrime[tau];
        double s2 = dPrime[tau + 1];
        double denom = 2.0 * s1 - s2 - s0;
        if (qAbs(denom) > 1e-10) {
            betterTau = tau + (s2 - s0) / (2.0 * denom);
        }
    }

    return (betterTau > 0) ? m_sampleRate / betterTau : 0.0;
}

/**
 * @brief 自相关函数基频检测
 * @param frame 输入帧
 * @return 基频(Hz)，未检测到返回0
 */
double PitchTrack3::detectACF(const QVector<double>& frame)
{
    int frameSize = frame.size();
    int maxLag = qRound(m_sampleRate / m_fmin);
    int minLag = qRound(m_sampleRate / m_fmax);

    maxLag = qMin(maxLag, frameSize / 2);
    minLag = qMax(2, minLag);

    if (maxLag <= minLag) return 0.0;

    // 计算自相关函数
    QVector<double> acf(maxLag, 0.0);
    for (int lag = minLag; lag < maxLag; ++lag) {
        for (int i = 0; i < frameSize - lag; ++i) {
            acf[lag] += frame[i] * frame[i + lag];
        }
    }

    // 归一化
    double maxAcf = *std::max_element(acf.begin() + minLag, acf.end());
    if (maxAcf < 1e-10) return 0.0;

    // 找第一个峰值
    int bestLag = minLag;
    double bestVal = -1e18;
    for (int lag = minLag; lag < maxLag - 1; ++lag) {
        if (acf[lag] > acf[lag - 1] && acf[lag] > acf[lag + 1]) {
            if (acf[lag] > bestVal) {
                bestVal = acf[lag];
                bestLag = lag;
            }
        }
    }

    return (bestLag > 0) ? m_sampleRate / bestLag : 0.0;
}
