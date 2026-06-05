#include "StereoWidener5.h"
#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/**
 * @brief 构造函数，初始化立体声展宽处理器
 * @param parent 父对象指针
 */
StereoWidener5::StereoWidener5(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 重置所有统计信息
 */
void StereoWidener5::resetStatistics()
{
    m_stats = Stats();
    m_timeSum = 0.0;
}

/**
 * @brief 设置展宽宽度
 * @param width 展宽系数(0.0=单声道, 1.0=正常, 2.0=超宽)
 */
void StereoWidener5::setWidth(double width)
{
    m_width = qBound(0.0, width, 3.0);
}

/**
 * @brief 设置低频保护截止频率
 *
 * 低于此频率的信号不进行展宽处理，以保持低频居中感。
 *
 * @param freqHz 截止频率(Hz)
 */
void StereoWidener5::setLowFrequencyProtection(double freqHz)
{
    m_lpfFreq = qMax(20.0, freqHz);
}

/**
 * @brief 对立体声帧对执行展宽处理
 *
 * 使用M/S（中/侧）编码技术：
 * 1. L/R转M/S编码：M=(L+R)/2, S=(L-R)/2
 * 2. 调整S通道增益：S *= width
 * 3. 低频保护：对低频分量减少展宽量
 * 4. M/S转L/R解码：L=M+S, R=M-S
 *
 * @param stereoFrames 立体声帧对(L, R)
 * @return 展宽后的立体声帧对
 */
QVector<QPair<double, double>> StereoWidener5::process(
    const QVector<QPair<double, double>>& stereoFrames)
{
    QElapsedTimer timer;
    timer.start();

    const int n = stereoFrames.size();
    QVector<QPair<double, double>> output(n);
    if (n == 0) {
        emit processingCompleted(0);
        return output;
    }

    /* M/S展宽系数：width映射到侧通道增益 */
    const double sideGain = m_width;
    /* 低频保护系数：低频以下展宽量衰减到1.0 */
    const double lpfRatio = m_lpfFreq / 22050.0;
    /* 简化低频保护：前lpfRatio比例的样本减少展宽 */
    const int lpfZone = static_cast<int>(lpfRatio * n);

    for (int i = 0; i < n; ++i) {
        double L = stereoFrames[i].first;
        double R = stereoFrames[i].second;

        /* M/S编码 */
        double M = (L + R) * 0.5;
        double S = (L - R) * 0.5;

        /* 低频保护：对低频区域减小展宽量 */
        double localGain = sideGain;
        if (i < lpfZone && lpfZone > 0) {
            double blend = static_cast<double>(i) / lpfZone;
            localGain = 1.0 + (sideGain - 1.0) * blend;
        }

        /* 展宽：调整侧通道增益 */
        S *= localGain;

        /* M/S解码 */
        output[i].first = M + S;
        output[i].second = M - S;
    }

    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.totalProcessed++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalProcessed;

    emit processingCompleted(n);
    return output;
}
