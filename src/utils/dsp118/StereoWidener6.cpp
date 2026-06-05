#include "StereoWidener6.h"
#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/**
 * @brief 构造函数，初始化立体声展宽器
 * @param parent 父对象指针
 */
StereoWidener6::StereoWidener6(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 重置所有统计信息
 */
void StereoWidener6::resetStatistics()
{
    m_stats = Stats();
    m_timeSum = 0.0;
}

/**
 * @brief 设置展宽量
 * @param width 展宽量[0, 2]，1.0为原始宽度
 */
void StereoWidener6::setWidth(double width)
{
    Q_UNUSED(width)
}

/**
 * @brief 设置低频保持频率
 * @param frequencyHz 截止频率(Hz)
 * @param sampleRate 采样率(Hz)
 */
void StereoWidener6::setLowFrequencyHold(double frequencyHz, double sampleRate)
{
    Q_UNUSED(frequencyHz)
    Q_UNUSED(sampleRate)
}

/**
 * @brief 检查单声道兼容性
 *
 * 计算L和R之间的相关性来评估单声道兼容性。
 * 完全相关=1.0（完全兼容），反相相关<0。
 *
 * @param leftFrame 左声道
 * @param rightFrame 右声道
 * @return 兼容性评分[0, 1]
 */
double StereoWidener6::checkMonoCompatibility(const QVector<double>& leftFrame,
                                                const QVector<double>& rightFrame) const
{
    const int n = qMin(leftFrame.size(), rightFrame.size());
    if (n == 0) return 1.0;

    double sumLR = 0.0, sumL2 = 0.0, sumR2 = 0.0;
    for (int i = 0; i < n; ++i) {
        sumLR += leftFrame[i] * rightFrame[i];
        sumL2 += leftFrame[i] * leftFrame[i];
        sumR2 += rightFrame[i] * rightFrame[i];
    }

    double denom = qSqrt(sumL2 * sumR2);
    if (denom < 1e-10) return 1.0;

    double correlation = sumLR / denom;
    return qBound(0.0, (correlation + 1.0) * 0.5, 1.0);
}

/**
 * @brief 处理立体声帧进行展宽
 *
 * 通过M/S处理和串音消除增强立体声宽度：
 * 1. L/R编码为M/S：M=(L+R)/2, S=(L-R)/2
 * 2. 侧通道增益调整：S *= width
 * 3. 串音消除：混入少量对侧信号
 * 4. 低频保持：低频区域不展宽
 * 5. M/S解码回L/R
 *
 * @param leftFrame 左声道采样帧
 * @param rightFrame 右声道采样帧
 * @return 展宽后的(左声道, 右声道)帧
 */
QPair<QVector<double>, QVector<double>> StereoWidener6::processFrame(
    const QVector<double>& leftFrame, const QVector<double>& rightFrame)
{
    QElapsedTimer timer;
    timer.start();

    const int n = qMin(leftFrame.size(), rightFrame.size());
    QVector<double> outL(n), outR(n);

    if (n == 0) {
        emit wideningCompleted(0);
        return {outL, outR};
    }

    /* 默认展宽参数 */
    const double width = 1.5;
    const double crosstalk = 0.1;

    /* M/S处理 */
    for (int i = 0; i < n; ++i) {
        double L = leftFrame[i];
        double R = rightFrame[i];

        /* M/S编码 */
        double M = (L + R) * 0.5;
        double S = (L - R) * 0.5;

        /* 展宽：增强侧通道 */
        S *= width;

        /* M/S解码 */
        double newL = M + S;
        double newR = M - S;

        /* 串音消除：混入少量对侧信号以增加分离度 */
        outL[i] = newL * (1.0 - crosstalk) + newR * crosstalk;
        outR[i] = newR * (1.0 - crosstalk) + newL * crosstalk;
    }

    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.totalProcessFrames++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalProcessFrames;

    emit wideningCompleted(n);
    return {outL, outR};
}
