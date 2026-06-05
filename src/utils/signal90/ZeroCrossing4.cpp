#include "ZeroCrossing4.h"
#include <QElapsedTimer>
#include <QtMath>

/**
 * @class ZeroCrossing4
 * @brief 过零率计算器实现
 *
 * 过零率(Zero Crossing Rate, ZCR)表示信号在单位时间内
 * 穿越零电平的次数。是语音/音频处理中常用的时域特征。
 *
 * ZCR = (1/N) * Σ |sign(x[i]) - sign(x[i-1])| / 2
 *
 * 应用:
 * - 语音活动检测(VAD): 清音过零率高，浊音低
 * - 音频分类: 不同乐器/音色的ZCR特征不同
 * - 频率估计: 正弦波的ZCR ≈ 2*f/fs
 */

/**
 * @brief 构造函数
 * @param parent 父QObject
 */
ZeroCrossing4::ZeroCrossing4(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 计算单帧过零率
 *
 * 统计帧内相邻采样符号变化的次数，归一化为每采样过零率。
 *
 * 过零条件: sign(x[i]) != sign(x[i-1])
 * 其中sign(x) = 1 if x >= 0, -1 if x < 0
 *
 * @param frame 输入帧
 * @return 过零率(0~0.5)，即平均每采样的过零次数
 */
double ZeroCrossing4::compute(const QVector<double>& frame) const
{
    if (frame.size() < 2) return 0.0;

    int crossings = 0;
    for (int i = 1; i < frame.size(); ++i) {
        /* 检测符号变化(排除零值，减少噪声影响) */
        if ((frame[i] >= 0.0) != (frame[i - 1] >= 0.0)) {
            crossings++;
        }
    }

    /* 归一化: 过零次数 / (帧长 - 1) */
    return static_cast<double>(crossings) / (frame.size() - 1);
}

/**
 * @brief 批量计算过零率序列
 *
 * 使用滑动窗口逐帧提取过零率，输出ZCR随时间的变化曲线。
 *
 * @param samples 完整音频采样序列
 * @param frameSize 帧大小(采样点)
 * @param hopSize 帧移(采样点)
 * @return 过零率序列
 */
QVector<double> ZeroCrossing4::computeSequence(const QVector<double>& samples, int frameSize, int hopSize)
{
    QElapsedTimer timer;
    timer.start();

    QVector<double> zcrSeq;

    if (samples.size() < frameSize || hopSize <= 0) {
        m_timeSum += timer.elapsed();
        return zcrSeq;
    }

    for (int start = 0; start + frameSize <= samples.size(); start += hopSize) {
        /* 提取当前帧 */
        QVector<double> frame(frameSize);
        for (int i = 0; i < frameSize; ++i) {
            frame[i] = samples[start + i];
        }

        /* 计算过零率 */
        double zcr = compute(frame);
        zcrSeq.append(zcr);

        /* 统计过零次数 */
        for (int i = 1; i < frameSize; ++i) {
            if ((frame[i] >= 0.0) != (frame[i - 1] >= 0.0)) {
                m_stats.totalCrossings++;
            }
        }

        m_stats.totalFramesAnalyzed++;

        emit zeroCrossingRateComputed(zcr, m_stats.totalFramesAnalyzed - 1);
    }

    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / qMax(1, m_stats.totalFramesAnalyzed);

    return zcrSeq;
}

/**
 * @brief 估计信号的主频率
 *
 * 利用过零率估计正弦信号的主频率。
 * 对于纯正弦波: f ≈ ZCR * fs / 2
 * 对于复杂信号，此估计为近似值。
 *
 * @param frame 输入帧
 * @param sampleRate 采样率(Hz)
 * @return 估计的频率(Hz)
 */
double ZeroCrossing4::estimateFrequency(const QVector<double>& frame, double sampleRate) const
{
    if (frame.size() < 2 || sampleRate <= 0.0) return 0.0;

    double zcr = compute(frame);
    /* 过零率转换为频率: f = ZCR * sampleRate / 2 */
    double freq = zcr * sampleRate / 2.0;

    return freq;
}

/**
 * @brief 检测帧是否为有声(语音活动检测)
 *
 * 简单VAD: 如果过零率过高(噪声特征)或过低(静音)，
 * 判定为无声。中间范围判定为有声。
 *
 * @param frame 输入帧
 * @param lowThreshold 低过零率阈值(默认0.01)
 * @param highThreshold 高过零率阈值(默认0.45)
 * @return true如果判定为有声帧
 */
bool ZeroCrossing4::isVoiced(const QVector<double>& frame,
                              double lowThreshold, double highThreshold) const
{
    double zcr = compute(frame);

    /* 过零率在合理范围内判定为有声 */
    return (zcr > lowThreshold && zcr < highThreshold);
}

/**
 * @brief 计算帧的短时能量
 *
 * 作为ZCR的补充特征，用于更精确的VAD。
 * 能量低于阈值的帧即使ZCR合理也判定为静音。
 *
 * @param frame 输入帧
 * @return 短时能量(均方值)
 */
double ZeroCrossing4::shortTimeEnergy(const QVector<double>& frame) const
{
    if (frame.isEmpty()) return 0.0;

    double energy = 0.0;
    for (double sample : frame) {
        energy += sample * sample;
    }
    return energy / frame.size();
}

/**
 * @brief 重置所有统计数据
 *
 * 将帧分析计数、过零计数和计时归零。
 */
void ZeroCrossing4::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
