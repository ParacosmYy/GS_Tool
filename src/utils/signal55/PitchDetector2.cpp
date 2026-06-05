/**
 * @file PitchDetector2.cpp
 * @brief 音高检测器实现，支持ACF自相关和YIN算法
 *
 * 实现了两种基频检测算法：
 * 1. ACF（自相关函数法）：通过寻找自相关函数峰值确定周期
 * 2. YIN算法：基于差分函数的改进方法，具有更好的精度
 *
 * 适用于音频信号处理、乐器调音、语音分析等场景。
 *
 * @author EmbedDebug Team
 * @date 2026-06-05
 */

#include "utils/signal55/PitchDetector2.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/**
 * @brief 构造函数，初始化默认参数
 * @param parent 父QObject对象指针
 */
PitchDetector2::PitchDetector2(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置采样率
 * @param sr 采样率，单位Hz
 */
void PitchDetector2::setSampleRate(double sr)
{
    m_sampleRate = qMax(1.0, sr);
}

/**
 * @brief 设置检测方法
 * @param method 方法名："acf"或"yin"
 */
void PitchDetector2::setMethod(const QString& method)
{
    if (method == "acf" || method == "yin")
        m_method = method;
}

/**
 * @brief ACF自相关法检测基频
 *
 * 通过计算信号的自相关函数，寻找第一个显著峰值的位置。
 * 峰值位置对应信号的周期，从而计算基频。
 *
 * @param frame 输入音频帧
 * @return 检测到的基频(Hz)，未检测到返回-1
 */
double PitchDetector2::detectACF(const QVector<double>& frame) const
{
    const int N = frame.size();
    if (N < 64) return -1.0;

    /* 计算自相关函数 */
    QVector<double> acf(N / 2, 0.0);
    for (int lag = 0; lag < N / 2; ++lag) {
        double sum = 0.0;
        for (int i = 0; i < N - lag; ++i)
            sum += frame[i] * frame[i + lag];
        acf[lag] = sum;
    }

    /* 寻找第一个过零点后的最大峰值 */
    double maxAcf = 0.0;
    int maxLag = -1;

    /* 跳过lag=0附近的区域，寻找第一个下降后的上升峰 */
    int minLag = static_cast<int>(m_sampleRate / 1000.0); /* 最低1000Hz */
    int maxLagLimit = static_cast<int>(m_sampleRate / 50.0); /* 最高50Hz */

    for (int lag = minLag; lag < qMin(N / 2, maxLagLimit); ++lag) {
        if (acf[lag] > maxAcf) {
            maxAcf = acf[lag];
            maxLag = lag;
        }
    }

    if (maxLag <= 0 || acf[0] < 1e-10) return -1.0;

    /* 检查峰值显著性 */
    double confidence = maxAcf / acf[0];
    if (confidence < 0.3) return -1.0;

    /* 抛物线插值提高精度 */
    if (maxLag > 0 && maxLag < N / 2 - 1) {
        double y0 = acf[maxLag - 1];
        double y1 = acf[maxLag];
        double y2 = acf[maxLag + 1];
        double delta = (y0 - 2.0 * y1 + y2);
        if (qAbs(delta) > 1e-10) {
            double refinement = (y0 - y2) / (2.0 * delta);
            if (qAbs(refinement) < 1.0)
                maxLag += refinement;
        }
    }

    return m_sampleRate / maxLag;
}

/**
 * @brief YIN算法检测基频
 *
 * YIN算法步骤：
 * 1. 计算差分函数 d(tau)
 * 2. 计算累积均值归一化差分函数 d'(tau)
 * 3. 寻找d'(tau)的绝对阈值交叉点
 * 4. 抛物线插值精确化周期估计
 *
 * @param frame 输入音频帧
 * @return 检测到的基频(Hz)，未检测到返回-1
 */
double PitchDetector2::detectYIN(const QVector<double>& frame) const
{
    const int N = frame.size();
    if (N < 64) return -1.0;

    int halfN = N / 2;

    /* Step 1: 差分函数 */
    QVector<double> diff(halfN, 0.0);
    for (int tau = 0; tau < halfN; ++tau) {
        double sum = 0.0;
        for (int i = 0; i < halfN; ++i)
            sum += (frame[i] - frame[i + tau]) * (frame[i] - frame[i + tau]);
        diff[tau] = sum;
    }

    /* Step 2: 累积均值归一化差分函数 */
    QVector<double> cmndf(halfN, 0.0);
    cmndf[0] = 1.0;
    double runningSum = 0.0;
    for (int tau = 1; tau < halfN; ++tau) {
        runningSum += diff[tau];
        cmndf[tau] = (runningSum > 1e-10) ? diff[tau] * tau / runningSum : 1.0;
    }

    /* Step 3: 绝对阈值法寻找周期 */
    double threshold = 0.15;
    int minLag = static_cast<int>(m_sampleRate / 1000.0); /* 最高1kHz */
    int bestTau = -1;

    for (int tau = minLag; tau < halfN; ++tau) {
        if (cmndf[tau] < threshold) {
            /* 找到局部最小值 */
            while (tau + 1 < halfN && cmndf[tau + 1] < cmndf[tau])
                ++tau;
            bestTau = tau;
            break;
        }
    }

    /* 如果未找到阈值交叉，使用全局最小值 */
    if (bestTau < 0) {
        double minVal = 1e10;
        for (int tau = minLag; tau < halfN; ++tau) {
            if (cmndf[tau] < minVal) {
                minVal = cmndf[tau];
                bestTau = tau;
            }
        }
        if (bestTau < 0 || minVal > 0.5) return -1.0;
    }

    /* Step 4: 抛物线插值 */
    double refinedTau = bestTau;
    if (bestTau > 0 && bestTau < halfN - 1) {
        double y0 = cmndf[bestTau - 1];
        double y1 = cmndf[bestTau];
        double y2 = cmndf[bestTau + 1];
        double delta = y0 - 2.0 * y1 + y2;
        if (qAbs(delta) > 1e-10) {
            double refinement = (y0 - y2) / (2.0 * delta);
            if (qAbs(refinement) < 1.0)
                refinedTau = bestTau + refinement;
        }
    }

    if (refinedTau < 1.0) return -1.0;

    double freq = m_sampleRate / refinedTau;
    double confidence = 1.0 - cmndf[bestTau];

    Q_UNUSED(confidence);
    return freq;
}

/**
 * @brief 检测单帧音频的基频
 * @param frame 输入音频帧
 * @return 基频(Hz)，未检测到返回-1
 */
double PitchDetector2::detect(const QVector<double>& frame)
{
    QElapsedTimer timer;
    timer.start();

    double freq = -1.0;
    if (m_method == "acf")
        freq = detectACF(frame);
    else
        freq = detectYIN(frame);

    /* 更新统计 */
    m_stats.totalDetections++;
    m_stats.totalFrames++;
    double elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalFrames;

    double confidence = (freq > 0) ? 0.8 : 0.0;
    emit pitchDetected(freq, confidence);

    return freq;
}

/**
 * @brief 检测信号序列的基频轨迹
 * @param signal 完整音频信号
 * @param frameSize 帧大小（样本数）
 * @param hop 帧移（样本数）
 * @return 基频轨迹列表（频率, 置信度对）
 */
QVector<QPair<double, double>> PitchDetector2::detectSequence(const QVector<double>& signal,
                                                                int frameSize, int hop)
{
    QElapsedTimer timer;
    timer.start();

    QVector<QPair<double, double>> result;

    for (int start = 0; start + frameSize <= signal.size(); start += hop) {
        QVector<double> frame(frameSize);
        for (int i = 0; i < frameSize; ++i)
            frame[i] = signal[start + i];

        double freq = (m_method == "acf") ? detectACF(frame) : detectYIN(frame);
        double confidence = (freq > 0) ? 0.8 : 0.0;
        result.append({freq, confidence});
    }

    m_stats.totalFrames += result.size();
    double elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalFrames;

    return result;
}

/**
 * @brief 重置所有统计数据
 */
void PitchDetector2::resetStatistics()
{
    m_stats = Stats();
    m_timeSum = 0.0;
}
