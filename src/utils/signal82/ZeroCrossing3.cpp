/**
 * @file ZeroCrossing3.cpp
 * @brief 过零率分析器实现
 *
 * 实现基于过零检测的信号分析方法:
 * 1. 过零率(ZCR): 信号穿越零电平的频率
 *    ZCR = crossCount / frameLength
 *    高ZCR -> 高频信号或噪声; 低ZCR -> 低频信号
 *
 * 2. 频率估计: 根据正到负过零间隔估计基频
 *    freq = sampleRate / medianPeriod
 *    使用中位数(非均值)提高对异常值的鲁棒性
 *
 * 3. 有声/无声判别: ZCR > threshold -> 无声/噪声
 *    用于语音活动检测(VAD)的初级分类
 *
 * 4. 过零位置插值: 线性插值精确定位过零点
 *    crossPos = i + |x[i]| / |x[i+1] - x[i]|
 *    提供亚采样精度的过零位置
 *
 * @author EmbedDebug Team
 * @version 1.0
 * @date 2026-06-06
 */

#include "utils/signal82/ZeroCrossing3.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/**
 * @brief 构造函数，初始化过零率分析器
 * @param parent 父QObject指针
 *
 * 默认采样率44100Hz。
 */
ZeroCrossing3::ZeroCrossing3(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置采样率
 *
 * 采样率用于将过零间隔(采样数)转换为频率(Hz)。
 *
 * @param sampleRate 采样率(Hz)，必须大于0
 */
void ZeroCrossing3::setSampleRate(double sampleRate)
{
    m_sampleRate = qMax(1.0, sampleRate);
}

/**
 * @brief 计算一帧的过零率(交叉次数/帧长)
 *
 * 遍历帧内所有相邻采样对，统计符号不同的次数。
 * 过零率 = crossingCount / frameLength
 *
 * 过零率的典型范围:
 * - 纯正弦波: 约 2 * freq / sampleRate
 * - 白噪声: 约 0.5 (理论值)
 * - 浊语音: 0.01 ~ 0.1 (低ZCR)
 * - 清语音: 0.1 ~ 0.5 (高ZCR)
 *
 * @param frame 输入音频帧
 * @return 过零率[0, 1]，0表示无过零或空帧
 */
double ZeroCrossing3::zeroCrossingRate(const QVector<double>& frame) const
{
    if (frame.size() < 2) {
        return 0.0;
    }

    const int N = frame.size();
    int crossings = 0;

    for (int i = 0; i < N - 1; ++i) {
        /* 检测符号变化(一个为正一个为负，或一个为零一个非零) */
        if ((frame[i] >= 0.0 && frame[i + 1] < 0.0) ||
            (frame[i] < 0.0 && frame[i + 1] >= 0.0)) {
            crossings++;
        }
    }

    return static_cast<double>(crossings) / static_cast<double>(N - 1);
}

/**
 * @brief 计算过零点对应的估计频率(Hz)
 *
 * 仅统计正到负方向的过零(更稳定的基频估计):
 * 1. 检测所有正到负过零点
 * 2. 线性插值精确定位过零位置
 * 3. 计算相邻过零点间隔(半周期)
 * 4. 半周期 * 2 = 完整周期
 * 5. freq = sampleRate / medianPeriod
 *
 * 退避策略:
 * - 2个以上过零点: 使用间隔中位数
 * - 1个过零点: freq = 1 / (2 * pos / sampleRate)
 * - 0个过零点: freq = 0
 *
 * @param frame 输入音频帧
 * @return 估计频率(Hz)，限制在[0, Nyquist]
 */
double ZeroCrossing3::estimatedFrequency(const QVector<double>& frame) const
{
    if (frame.size() < 2) {
        return 0.0;
    }

    const int N = frame.size();

    /* 检测正到负方向的过零点并线性插值精确定位 */
    QVector<double> posCrossings;

    for (int i = 0; i < N - 1; ++i) {
        /* 正到负穿越 */
        if (frame[i] >= 0.0 && frame[i + 1] < 0.0) {
            double crossPos;
            double diff = frame[i + 1] - frame[i];
            if (qAbs(diff) > 1e-15) {
                /* 线性插值: crossPos = i + |frame[i]| / |frame[i+1] - frame[i]| */
                crossPos = i + qAbs(frame[i]) / qAbs(diff);
            } else {
                crossPos = i + 0.5;
            }
            posCrossings.append(crossPos);
        }
    }

    /* 基于过零间隔估计频率 */
    if (posCrossings.size() >= 2) {
        /* 计算相邻过零间隔(半周期) */
        QVector<double> halfPeriods;
        for (int i = 1; i < posCrossings.size(); ++i) {
            double halfP = posCrossings[i] - posCrossings[i - 1];
            if (halfP > 0.0) {
                halfPeriods.append(halfP);
            }
        }

        if (!halfPeriods.isEmpty()) {
            /* 取中位数(比均值更鲁棒) */
            std::sort(halfPeriods.begin(), halfPeriods.end());
            double medianHalfPeriod = halfPeriods[halfPeriods.size() / 2];
            /* 完整周期 = 2 * 半周期 */
            double freq = m_sampleRate / (2.0 * medianHalfPeriod);
            return qBound(0.0, freq, m_sampleRate / 2.0);
        }
    } else if (posCrossings.size() == 1) {
        /* 单个过零点: 粗略估计 */
        double halfPeriod = posCrossings[0];
        if (halfPeriod > 0.0) {
            return qBound(0.0, m_sampleRate / (2.0 * halfPeriod), m_sampleRate / 2.0);
        }
    }

    return 0.0;
}

/**
 * @brief 检测帧是否为有声(基于过零率阈值)
 *
 * 语音信号分类的基本方法:
 * - 有声(浊音): 低过零率，声带振动产生准周期信号
 * - 无声(清音/噪声): 高过零率，类似噪声的非周期信号
 *
 * 典型阈值:
 * - threshold = 0.1: 保守判别(更多帧被判为有声)
 * - threshold = 0.15: 常用值
 * - threshold = 0.2: 激进判别(更多帧被判为无声)
 *
 * @param frame 输入音频帧
 * @param threshold 过零率阈值(默认0.1)
 * @return true表示有声(浊音)，false表示无声(清音/噪声)
 */
bool ZeroCrossing3::isVoiced(const QVector<double>& frame, double threshold) const
{
    double zcr = zeroCrossingRate(frame);
    return zcr < threshold;
}

/**
 * @brief 获取过零点的精确位置(插值)
 *
 * 检测所有方向的过零点(正到负和负到正)，
 * 并使用线性插值精确定位到亚采样精度。
 *
 * 线性插值公式:
 * 当x[i]与x[i+1]异号时:
 * crossPos = i + |x[i]| / |x[i+1] - x[i]|
 *
 * @param frame 输入音频帧
 * @return 过零点位置向量(以采样为单位的小数位置)
 */
QVector<double> ZeroCrossing3::crossingPositions(const QVector<double>& frame) const
{
    QElapsedTimer timer;
    timer.start();

    QVector<double> positions;

    if (frame.size() < 2) {
        const_cast<ZeroCrossing3*>(this)->m_stats.totalFramesAnalyzed++;
        double elapsed = timer.elapsed();
        const_cast<ZeroCrossing3*>(this)->m_timeSum += elapsed;
        const_cast<ZeroCrossing3*>(this)->m_stats.avgProcessingTimeMs =
            (m_stats.totalFramesAnalyzed > 0)
            ? m_timeSum / m_stats.totalFramesAnalyzed : 0.0;
        emit crossingRateComputed(0.0, 0.0);
        return positions;
    }

    const int N = frame.size();

    /* 检测所有方向的过零点 */
    for (int i = 0; i < N - 1; ++i) {
        bool signChange = (frame[i] >= 0.0 && frame[i + 1] < 0.0) ||
                          (frame[i] < 0.0 && frame[i + 1] >= 0.0);

        if (signChange) {
            double crossPos;
            double diff = frame[i + 1] - frame[i];
            if (qAbs(diff) > 1e-15) {
                crossPos = i + qAbs(frame[i]) / qAbs(diff);
            } else {
                crossPos = i + 0.5;
            }
            positions.append(crossPos);
        }
    }

    /* 计算过零率和估计频率用于信号发射 */
    double zcr = static_cast<double>(positions.size()) / static_cast<double>(N - 1);
    double freq = estimatedFrequency(frame);

    /* 更新统计 */
    m_stats.totalFramesAnalyzed++;
    m_stats.totalZeroCrossings += positions.size();
    double elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = (m_stats.totalFramesAnalyzed > 0)
        ? m_timeSum / m_stats.totalFramesAnalyzed : 0.0;

    emit crossingRateComputed(zcr, freq);
    return positions;
}

/**
 * @brief 重置所有统计数据
 *
 * 清零帧计数、总过零数和平均处理时间。
 */
void ZeroCrossing3::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
