#include "PitchTrack7.h"
#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/**
 * @brief 构造函数，初始化音高追踪器
 * @param parent 父对象指针
 */
PitchTrack7::PitchTrack7(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置基频搜索范围
 * @param minHz 最低频率(Hz)
 * @param maxHz 最高频率(Hz)
 */
void PitchTrack7::setFrequencyRange(double minHz, double maxHz)
{
    m_minFreq = qMax(20.0, minHz);
    m_maxFreq = qMax(m_minFreq + 1.0, maxHz);
}

/**
 * @brief 设置帧参数
 * @param frameSize 帧长(样本数)
 * @param hopSize 跳步(样本数)
 */
void PitchTrack7::setFrameParams(int frameSize, int hopSize)
{
    m_frameSize = qMax(64, frameSize);
    m_hopSize = qMax(1, hopSize);
}

/**
 * @brief 执行基频追踪
 *
 * 对输入音频信号逐帧执行YIN算法基频检测：
 * 1. 分帧处理，每帧施加Hanning窗
 * 2. 对每帧计算YIN差分函数
 * 3. 累积均值归一化差分函数(CMNDF)
 * 4. 在允许的延迟范围内找到第一个低于阈值的谷值
 * 5. 抛物线插值精化基频估计
 * 6. 跳过无声帧(能量低于阈值)
 *
 * @param audio 输入音频信号
 * @return (帧索引, 基频Hz)序列，无声帧频率为0
 */
QVector<QPair<int, double>> PitchTrack7::track(const QVector<double>& audio)
{
    QElapsedTimer timer;
    timer.start();

    QVector<QPair<int, double>> pitchTrack;
    const int n = audio.size();
    if (n < m_frameSize) {
        emit trackingCompleted(0);
        return pitchTrack;
    }

    const int numFrames = (n - m_frameSize) / m_hopSize + 1;
    const int halfLen = m_frameSize / 2;

    /* 预计算Hanning窗 */
    QVector<double> window(m_frameSize);
    for (int i = 0; i < m_frameSize; ++i) {
        window[i] = 0.5 * (1.0 - qCos(2.0 * M_PI * i / (m_frameSize - 1)));
    }

    /* 延迟范围（对应频率范围） */
    const int minLag = qMax(2, static_cast<int>(44100.0 / m_maxFreq));
    const int maxLag = qMin(halfLen, static_cast<int>(44100.0 / m_minFreq));

    int voicedCount = 0;

    for (int f = 0; f < numFrames; ++f) {
        int start = f * m_hopSize;

        /* 提取加窗帧 */
        QVector<double> frame(m_frameSize);
        double energy = 0.0;
        for (int i = 0; i < m_frameSize && start + i < n; ++i) {
            frame[i] = audio[start + i] * window[i];
            energy += frame[i] * frame[i];
        }
        energy /= m_frameSize;

        /* 无声帧跳过 */
        if (energy < 1e-8) {
            pitchTrack.append(qMakePair(f, 0.0));
            continue;
        }

        /* YIN差分函数 */
        QVector<double> diff(halfLen + 1, 0.0);
        for (int tau = 0; tau <= halfLen; ++tau) {
            double sum = 0.0;
            for (int i = 0; i < halfLen; ++i) {
                double d = frame[i] - frame[i + tau];
                sum += d * d;
            }
            diff[tau] = sum;
        }

        /* 累积均值归一化差分函数(CMNDF) */
        QVector<double> cmndf(halfLen + 1, 0.0);
        cmndf[0] = 1.0;
        double runningSum = 0.0;
        for (int tau = 1; tau <= halfLen; ++tau) {
            runningSum += diff[tau];
            cmndf[tau] = (runningSum > 0) ? diff[tau] * tau / runningSum : 1.0;
        }

        /* 寻找最小谷值 */
        const double yinThreshold = 0.15;
        int bestLag = -1;
        double minVal = 1e18;

        for (int tau = minLag; tau <= maxLag; ++tau) {
            if (cmndf[tau] < yinThreshold && cmndf[tau] < minVal) {
                minVal = cmndf[tau];
                bestLag = tau;
            }
        }

        /* 如果没有找到低于阈值的谷值，取全局最小 */
        if (bestLag < 0) {
            for (int tau = minLag; tau <= maxLag; ++tau) {
                if (cmndf[tau] < minVal) {
                    minVal = cmndf[tau];
                    bestLag = tau;
                }
            }
        }

        double freq = 0.0;
        if (bestLag > 1 && bestLag < halfLen) {
            /* 抛物线插值精化 */
            double s0 = cmndf[bestLag - 1];
            double s1 = cmndf[bestLag];
            double s2 = cmndf[bestLag + 1];
            double shift = (s0 - s2) / (2.0 * (s0 - 2.0 * s1 + s2) + 1e-10);
            shift = qBound(-0.5, shift, 0.5);
            double refinedLag = bestLag + shift;
            freq = 44100.0 / refinedLag;

            if (freq < m_minFreq || freq > m_maxFreq) {
                freq = 0.0;
            } else {
                voicedCount++;
            }
        }

        pitchTrack.append(qMakePair(f, freq));
    }

    m_stats.totalTracked += voicedCount;

    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = (m_stats.totalTracked > 0)
        ? m_timeSum / m_stats.totalTracked : 0.0;

    emit trackingCompleted(voicedCount);
    return pitchTrack;
}

/**
 * @brief 重置所有统计信息
 */
void PitchTrack7::resetStatistics()
{
    m_stats = Stats();
    m_timeSum = 0.0;
}
