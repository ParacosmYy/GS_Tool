/**
 * @file PitchDetector3.cpp
 * @brief 基音频率检测器实现
 *
 * 结合自相关(ACF)和AMDF(平均幅度差函数)方法的基音检测，
 * 支持实时逐帧分析。输出频率(Hz)和置信度。
 * 适用于语音处理、音乐分析和基音跟踪。
 */

#include "utils/signal76/PitchDetector3.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/**
 * @brief 构造函数，初始化默认参数
 * @param parent 父对象指针
 *
 * 默认采样率44100Hz，搜索范围50-800Hz。
 */
PitchDetector3::PitchDetector3(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置采样率
 * @param sampleRate 采样率(Hz)，范围[8000, 192000]
 */
void PitchDetector3::setSampleRate(double sampleRate)
{
    m_sampleRate = qBound(8000.0, sampleRate, 192000.0);
}

/**
 * @brief 设置频率搜索范围
 * @param minHz 最低频率(Hz)
 * @param maxHz 最高频率(Hz)
 *
 * 搜索范围决定基音周期的搜索区间。
 * 人声: 50-500Hz, 乐器: 30-4000Hz
 */
void PitchDetector3::setFrequencyRange(double minHz, double maxHz)
{
    m_minHz = qBound(20.0, minHz, m_sampleRate / 4.0);
    m_maxHz = qBound(m_minHz, maxHz, m_sampleRate / 4.0);
}

/**
 * @brief 检测帧的基音频率
 * @param frame 输入音频帧(通常20-50ms)
 * @return 基音频率(Hz)，无基音返回0
 *
 * 使用自相关方法:
 * 1. 计算信号的自相关函数R(tau)
 * 2. 在搜索范围内找最大R(tau)
 * 3. 使用抛物线插值精确定位峰值
 * 4. 基音频率 = 采样率 / 精确周期
 */
double PitchDetector3::detectPitch(const QVector<double>& frame)
{
    QElapsedTimer timer;
    timer.start();

    if (frame.size() < 64) return 0.0;

    int N = frame.size();
    int minLag = qMax(2, static_cast<int>(m_sampleRate / m_maxHz));
    int maxLag = qMin(N / 2, static_cast<int>(m_sampleRate / m_minHz));

    if (minLag >= maxLag) return 0.0;

    /* 步骤1: 计算自相关函数 */
    QVector<double> acf(maxLag + 1, 0.0);
    for (int tau = 0; tau <= maxLag; ++tau) {
        double sum = 0.0;
        for (int i = 0; i < N - tau; ++i) {
            sum += frame[i] * frame[i + tau];
        }
        acf[tau] = sum;
    }

    /* 归一化: acf[0]为能量 */
    double energy = acf[0];
    if (energy < 1e-10) return 0.0;

    /* 步骤2: 找自相关峰值(跳过零延迟) */
    int bestLag = minLag;
    double bestVal = -1e18;

    for (int tau = minLag; tau <= maxLag; ++tau) {
        if (acf[tau] > bestVal) {
            bestVal = acf[tau];
            bestLag = tau;
        }
    }

    /* 检查峰值是否足够显著(至少为能量的30%) */
    if (bestVal < 0.3 * energy) {
        m_lastPeriod = 0;

        qint64 elapsed = timer.elapsed();
        m_stats.totalFramesAnalyzed++;
        m_timeSum += elapsed;
        m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalFramesAnalyzed;

        return 0.0;
    }

    /* 步骤3: 抛物线插值精确定位 */
    double refinedLag = bestLag;
    if (bestLag > minLag && bestLag < maxLag) {
        double y0 = acf[bestLag - 1];
        double y1 = acf[bestLag];
        double y2 = acf[bestLag + 1];
        double denom = 2.0 * (2.0 * y1 - y0 - y2);
        if (qAbs(denom) > 1e-10) {
            refinedLag = bestLag + (y0 - y2) / denom;
        }
    }

    /* 步骤4: 计算基音频率 */
    double pitch = m_sampleRate / refinedLag;
    m_lastPeriod = qRound(refinedLag);

    /* 更新统计 */
    qint64 elapsed = timer.elapsed();
    m_stats.totalFramesAnalyzed++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalFramesAnalyzed;

    emit pitchDetected(pitch, bestVal / energy);
    return pitch;
}

/**
 * @brief 检测基音并返回频率和置信度
 * @param frame 输入音频帧
 * @return QPair(频率Hz, 置信度[0,1])
 *
 * 置信度 = 自相关峰值 / 能量，范围[0,1]。
 * 高置信度(>0.7)表示可靠的基音检测。
 */
QPair<double, double> PitchDetector3::detectPitchWithConfidence(const QVector<double>& frame)
{
    QElapsedTimer timer;
    timer.start();

    if (frame.size() < 64) return {0.0, 0.0};

    int N = frame.size();
    int minLag = qMax(2, static_cast<int>(m_sampleRate / m_maxHz));
    int maxLag = qMin(N / 2, static_cast<int>(m_sampleRate / m_minHz));

    if (minLag >= maxLag) return {0.0, 0.0};

    /* 计算自相关 */
    QVector<double> acf(maxLag + 1, 0.0);
    for (int tau = 0; tau <= maxLag; ++tau) {
        double sum = 0.0;
        for (int i = 0; i < N - tau; ++i) {
            sum += frame[i] * frame[i + tau];
        }
        acf[tau] = sum;
    }

    double energy = acf[0];
    if (energy < 1e-10) return {0.0, 0.0};

    /* 找峰值 */
    int bestLag = minLag;
    double bestVal = -1e18;
    for (int tau = minLag; tau <= maxLag; ++tau) {
        if (acf[tau] > bestVal) {
            bestVal = acf[tau];
            bestLag = tau;
        }
    }

    double confidence = bestVal / energy;

    if (confidence < 0.3) {
        m_lastPeriod = 0;
        return {0.0, confidence};
    }

    /* 抛物线插值 */
    double refinedLag = bestLag;
    if (bestLag > minLag && bestLag < maxLag) {
        double y0 = acf[bestLag - 1];
        double y1 = acf[bestLag];
        double y2 = acf[bestLag + 1];
        double denom = 2.0 * (2.0 * y1 - y0 - y2);
        if (qAbs(denom) > 1e-10) {
            refinedLag = bestLag + (y0 - y2) / denom;
        }
    }

    double pitch = m_sampleRate / refinedLag;
    m_lastPeriod = qRound(refinedLag);

    qint64 elapsed = timer.elapsed();
    m_stats.totalFramesAnalyzed++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalFramesAnalyzed;

    emit pitchDetected(pitch, confidence);
    return {pitch, confidence};
}

/**
 * @brief 获取上一帧的基音周期
 * @return 基音周期(样本数)，无基音返回0
 */
int PitchDetector3::lastPeriod() const
{
    return m_lastPeriod;
}

/**
 * @brief 重置统计信息
 */
void PitchDetector3::resetStatistics()
{
    m_stats = Stats();
    m_timeSum = 0.0;
}
