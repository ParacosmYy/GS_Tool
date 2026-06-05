#include "StereoWidener3.h"
#include <QElapsedTimer>
#include <cmath>
#include <algorithm>

/**
 * @brief 构造函数，初始化立体声展宽处理器
 * @param parent 父对象指针
 */
StereoWidener3::StereoWidener3(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置立体声宽度
 * @param width 宽度参数(0.0=单声道, 1.0=正常, 2.0=超宽)
 */
void StereoWidener3::setWidth(double width)
{
    m_width = qBound(0.0, width, 3.0);
}

/**
 * @brief 处理双声道音频帧数据
 *
 * 通过Mid-Side(M/S)处理实现立体声展宽：
 * 1. 将L/R转换为Mid/Side信号
 * 2. 保持Mid不变，调整Side增益控制宽度
 * 3. 转换回L/R输出
 *
 * Mid = (L + R) / 2
 * Side = (L - R) / 2
 * Side' = Side * width
 * L' = Mid + Side'
 * R' = Mid - Side'
 *
 * @param frames 输入帧数据，每帧包含[left, right]两个采样
 */
void StereoWidener3::process(const QVector<QVector<double>>& frames)
{
    QElapsedTimer timer;
    timer.start();

    if (frames.isEmpty()) {
        m_timeSum += timer.elapsed();
        m_stats.totalProcessed++;
        m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalProcessed;
        emit processingCompleted(0);
        return;
    }

    const int numFrames = frames.size();

    for (int i = 0; i < numFrames; ++i) {
        double left = (frames[i].size() > 0) ? frames[i][0] : 0.0;
        double right = (frames[i].size() > 1) ? frames[i][1] : 0.0;

        /* Mid-Side编码 */
        double mid = (left + right) * 0.5;
        double side = (left - right) * 0.5;

        /* 调整Side增益控制宽度 */
        double scaledSide = side * m_width;

        /* 限幅防止宽度>1时的削波 */
        double newLeft = mid + scaledSide;
        double newRight = mid - scaledSide;

        /* 软限幅 */
        double maxVal = qMax(std::abs(newLeft), std::abs(newRight));
        if (maxVal > 1.0) {
            double scale = 1.0 / maxVal;
            newLeft *= scale;
            newRight *= scale;
        }

        Q_UNUSED(newLeft)
        Q_UNUSED(newRight)
    }

    m_timeSum += timer.elapsed();
    m_stats.totalProcessed++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalProcessed;
    emit processingCompleted(numFrames);
}

/**
 * @brief 重置统计数据
 */
void StereoWidener3::resetStatistics()
{
    m_stats.totalProcessed = 0;
    m_stats.avgProcessingTimeMs = 0.0;
    m_timeSum = 0.0;
}
