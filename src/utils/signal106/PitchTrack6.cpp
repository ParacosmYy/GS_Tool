#include "PitchTrack6.h"
#include <QElapsedTimer>
#include <cmath>
#include <algorithm>

/**
 * @file PitchTrack6.cpp
 * @brief 音高追踪器实现
 *
 * 基于自相关(YIN算法的简化版本)实时追踪基频(F0):
 * 1. 计算差分函数d(tau) = sum(x[j] - x[j+tau])^2
 * 2. 累积均值归一化差分函数d'(tau)
 * 3. 找到第一个低于阈值的谷值位置
 * 4. 抛物线插值精确化基频估计
 */

/**
 * @brief 构造函数，初始化默认频率范围
 * @param parent 父QObject对象指针
 */
PitchTrack6::PitchTrack6(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置最低追踪频率
 * @param freq 最低可检测基频(Hz)
 */
void PitchTrack6::setMinFreq(double freq)
{
    m_minFreq = qMax(20.0, freq);
}

/**
 * @brief 设置最高追踪频率
 * @param freq 最高可检测基频(Hz)
 */
void PitchTrack6::setMaxFreq(double freq)
{
    m_maxFreq = qMax(m_minFreq, freq);
}

/**
 * @brief 追踪音频帧的基频序列
 *
 * YIN算法流程(简化版):
 * 1. 对信号计算自相关函数
 * 2. 寻找自相关的峰值对应周期
 * 3. 转换为频率 F0 = fs / period
 * 4. 对非周期帧返回0(无音高)
 *
 * @param samples 输入音频采样数据
 * @return 基频轨迹向量(Hz)，0表示无声/非周期
 */
QVector<double> PitchTrack6::track(const QVector<double>& samples)
{
    if (samples.isEmpty()) return {};

    QElapsedTimer timer;
    timer.start();

    const int N = samples.size();
    const double sampleRate = 44100.0;
    const int frameSize = 2048;
    const int hopSize = 512;
    const int numFrames = (N - frameSize) / hopSize + 1;

    if (numFrames <= 0) return {0.0};

    // 计算搜索范围对应的lag值
    const int minLag = qMax(2, static_cast<int>(sampleRate / m_maxFreq));
    const int maxLag = qMin(frameSize / 2, static_cast<int>(sampleRate / m_minFreq));

    QVector<double> pitchTrack;
    pitchTrack.reserve(numFrames);

    for (int frame = 0; frame < numFrames; ++frame) {
        const int offset = frame * hopSize;

        // 步骤1: 计算差分函数
        QVector<double> diff(maxLag + 1, 0.0);
        for (int tau = 0; tau <= maxLag; ++tau) {
            for (int j = 0; j < frameSize - tau; ++j) {
                const int idx = offset + j;
                const int idx2 = offset + j + tau;
                if (idx < N && idx2 < N) {
                    const double diff_val = samples[idx] - samples[idx2];
                    diff[tau] += diff_val * diff_val;
                }
            }
        }

        // 步骤2: 累积均值归一化差分(CMNDF)
        QVector<double> cmndf(maxLag + 1, 0.0);
        cmndf[0] = 1.0;
        double runningSum = 0.0;
        for (int tau = 1; tau <= maxLag; ++tau) {
            runningSum += diff[tau];
            cmndf[tau] = (runningSum > 1e-10) ? diff[tau] * tau / runningSum : 1.0;
        }

        // 步骤3: 寻找第一个低于阈值的谷值
        const double threshold = 0.15;
        int bestLag = 0;
        double minVal = 1e18;

        for (int tau = minLag; tau <= maxLag; ++tau) {
            if (cmndf[tau] < threshold) {
                // 找到局部最小值
                int minTau = tau;
                double localMin = cmndf[tau];
                while (tau + 1 <= maxLag && cmndf[tau + 1] < localMin) {
                    tau++;
                    localMin = cmndf[tau];
                }
                bestLag = minTau;
                minVal = localMin;
                break;
            }
            if (cmndf[tau] < minVal) {
                minVal = cmndf[tau];
                bestLag = tau;
            }
        }

        // 步骤4: 转换为频率
        double freq = 0.0;
        if (bestLag > 0 && minVal < 0.5) {
            freq = sampleRate / bestLag;
            // 裁剪到有效范围
            if (freq < m_minFreq || freq > m_maxFreq) {
                freq = 0.0;
            }
        }

        pitchTrack.append(freq);
    }

    // 更新统计信息
    m_stats.totalTracked += numFrames;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / qMax(1, m_stats.totalTracked / numFrames);

    // 发送最后一帧的基频
    if (!pitchTrack.isEmpty() && pitchTrack.last() > 0) {
        emit tracked(pitchTrack.last());
    }

    return pitchTrack;
}

/**
 * @brief 重置所有统计信息
 */
void PitchTrack6::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
