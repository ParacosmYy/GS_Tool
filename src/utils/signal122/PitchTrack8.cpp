#include "PitchTrack8.h"
#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/**
 * @brief 构造函数，初始化音高追踪器
 * @param parent 父对象指针
 */
PitchTrack8::PitchTrack8(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 重置所有统计信息
 */
void PitchTrack8::resetStatistics()
{
    m_stats = Stats();
    m_timeSum = 0.0;
}

/**
 * @brief 设置音高检测范围
 * @param minFreqHz 最低频率(Hz)
 * @param maxFreqHz 最高频率(Hz)
 */
void PitchTrack8::setFrequencyRange(double minFreqHz, double maxFreqHz)
{
    Q_UNUSED(minFreqHz)
    Q_UNUSED(maxFreqHz)
}

/**
 * @brief 使用YIN算法检测音高
 *
 * YIN算法步骤：
 * 1. 计算差分函数 d(tau) = sum((x[i]-x[i+tau])^2)
 * 2. 累积均值归一化差分函数 d'(tau) = d(tau) / ((1/tau)*sum(d(j)))
 * 3. 绝对阈值法寻找谷值
 * 4. 抛物线插值精化
 *
 * @param frame 音频帧数据
 * @param sampleRate 采样率(Hz)
 * @return 检测到的基频(Hz)，无音调返回0
 */
double PitchTrack8::detectYIN(const QVector<double>& frame, double sampleRate)
{
    QElapsedTimer timer;
    timer.start();

    const int n = frame.size();
    const int halfLen = n / 2;

    if (n < 16 || sampleRate <= 0) {
        qint64 elapsed = timer.elapsed();
        m_timeSum += elapsed;
        m_stats.totalTrackOps++;
        m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalTrackOps;
        emit trackingCompleted(0);
        return 0.0;
    }

    /* 差分函数 */
    QVector<double> diff(halfLen + 1, 0.0);
    for (int tau = 0; tau <= halfLen; ++tau) {
        double sum = 0.0;
        for (int i = 0; i < halfLen; ++i) {
            double d = frame[i] - ((i + tau < n) ? frame[i + tau] : 0.0);
            sum += d * d;
        }
        diff[tau] = sum;
    }

    /* 累积均值归一化差分函数 */
    QVector<double> cmndf(halfLen + 1, 0.0);
    cmndf[0] = 1.0;
    double runningSum = 0.0;
    for (int tau = 1; tau <= halfLen; ++tau) {
        runningSum += diff[tau];
        cmndf[tau] = (runningSum > 0) ? diff[tau] * tau / runningSum : 1.0;
    }

    /* 寻找谷值 */
    const double yinThresh = 0.15;
    int bestLag = -1;
    double minVal = 1e18;
    for (int tau = 2; tau <= halfLen; ++tau) {
        if (cmndf[tau] < yinThresh) {
            /* 找到局部最小值 */
            int localMin = tau;
            while (localMin < halfLen && cmndf[localMin + 1] < cmndf[localMin]) {
                localMin++;
            }
            if (cmndf[localMin] < minVal) {
                minVal = cmndf[localMin];
                bestLag = localMin;
            }
            tau = localMin;
        }
    }

    double freq = 0.0;
    if (bestLag > 1 && bestLag < halfLen) {
        /* 抛物线插值 */
        double s0 = (bestLag > 0) ? cmndf[bestLag - 1] : cmndf[bestLag];
        double s1 = cmndf[bestLag];
        double s2 = (bestLag < halfLen) ? cmndf[bestLag + 1] : cmndf[bestLag];
        double denom = 2.0 * (s0 - 2.0 * s1 + s2);
        double shift = 0.0;
        if (qAbs(denom) > 1e-10) {
            shift = (s0 - s2) / denom;
            shift = qBound(-0.5, shift, 0.5);
        }
        double refinedLag = bestLag + shift;
        freq = sampleRate / refinedLag;
    }

    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.totalTrackOps++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalTrackOps;

    return freq;
}

/**
 * @brief 使用AMDF算法检测音高
 *
 * AMDF(Average Magnue Difference Function)：
 * D(tau) = sum(|x[i] - x[i+tau]|) / (N - tau)
 *
 * 在延迟范围内寻找AMDF最小值对应的延迟即为基频周期。
 *
 * @param frame 音频帧数据
 * @param sampleRate 采样率(Hz)
 * @return 检测到的基频(Hz)，无音调返回0
 */
double PitchTrack8::detectAMDF(const QVector<double>& frame, double sampleRate)
{
    QElapsedTimer timer;
    timer.start();

    const int n = frame.size();
    const int halfLen = n / 2;

    if (n < 16 || sampleRate <= 0) {
        qint64 elapsed = timer.elapsed();
        m_timeSum += elapsed;
        m_stats.totalTrackOps++;
        m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalTrackOps;
        return 0.0;
    }

    /* AMDF */
    QVector<double> amdf(halfLen + 1, 0.0);
    for (int tau = 0; tau <= halfLen; ++tau) {
        double sum = 0.0;
        int count = n - tau;
        for (int i = 0; i < count; ++i) {
            sum += qAbs(frame[i] - frame[i + tau]);
        }
        amdf[tau] = (count > 0) ? sum / count : 0.0;
    }

    /* 在有效延迟范围内搜索最小值 */
    int minLag = qMax(2, static_cast<int>(sampleRate / 2000.0));
    int maxLag = qMin(halfLen, static_cast<int>(sampleRate / 50.0));

    double minVal = 1e18;
    int bestLag = 0;
    for (int tau = minLag; tau <= maxLag; ++tau) {
        if (amdf[tau] < minVal) {
            minVal = amdf[tau];
            bestLag = tau;
        }
    }

    double freq = 0.0;
    if (bestLag > 1) {
        freq = sampleRate / bestLag;
    }

    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.totalTrackOps++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalTrackOps;

    return freq;
}

/**
 * @brief 批量音高追踪
 *
 * 对多帧音频数据逐帧使用YIN算法检测基频，
 * 返回每帧的基频估计值。
 *
 * @param frames 分帧后的音频数据
 * @param sampleRate 采样率(Hz)
 * @return 各帧的基频(Hz)
 */
QVector<double> PitchTrack8::trackSequence(
    const QVector<QVector<double>>& frames, double sampleRate)
{
    QElapsedTimer timer;
    timer.start();

    QVector<double> pitches;
    const int numFrames = frames.size();
    if (numFrames == 0) {
        emit trackingCompleted(0);
        return pitches;
    }

    pitches.reserve(numFrames);
    for (int i = 0; i < numFrames; ++i) {
        pitches.append(detectYIN(frames[i], sampleRate));
    }

    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.totalTrackOps += numFrames;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalTrackOps;

    emit trackingCompleted(numFrames);
    return pitches;
}
