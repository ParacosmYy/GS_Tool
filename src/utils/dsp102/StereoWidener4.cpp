#include "StereoWidener4.h"
#include <QElapsedTimer>
#include <cmath>
#include <algorithm>

/**
 * @file StereoWidener4.cpp
 * @brief 立体声展宽效果处理器实现
 *
 * 通过Mid-Side(M/S)处理实现立体声展宽:
 * - Mid = (L+R)/2 (中央信息)
 * - Side = (L-R)/2 (侧向信息)
 * 展宽通过增大Side/Mid比例实现。
 */

/**
 * @brief 构造函数，初始化默认宽度参数
 * @param parent 父QObject对象指针
 */
StereoWidener4::StereoWidener4(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置立体声宽度
 * @param width 宽度值: 0.0=单声道, 1.0=原始, 2.0=超宽
 */
void StereoWidener4::setWidth(double width)
{
    m_width = qBound(0.0, width, 3.0);
}

/**
 * @brief 处理立体声采样数据
 *
 * M/S处理流程:
 * 1. 将L/R转换为Mid/Side
 * 2. 按width系数缩放Side分量
 * 3. 将Mid/Side转换回L/R
 *
 * @param frames 输入立体声帧[left, right]对
 * @return 处理后的立体声帧
 */
QVector<QVector<double>> StereoWidener4::process(const QVector<QVector<double>>& frames)
{
    if (frames.isEmpty()) return frames;

    QElapsedTimer timer;
    timer.start();

    const int N = frames.size();
    QVector<QVector<double>> output(N);

    for (int i = 0; i < N; ++i) {
        if (frames[i].size() < 2) {
            output[i] = frames[i];
            continue;
        }

        const double L = frames[i][0];
        const double R = frames[i][1];

        // 步骤1: L/R -> Mid/Side
        const double mid = (L + R) * 0.5;
        const double side = (L - R) * 0.5;

        // 步骤2: 应用宽度系数到Side
        const double newSide = side * m_width;

        // 步骤3: Mid/Side -> L/R
        const double newL = mid + newSide;
        const double newR = mid - newSide;

        output[i] = {newL, newR};
    }

    // 更新统计信息
    m_stats.totalProcessed += N;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / qMax(1, m_stats.totalProcessed / N);

    emit processingCompleted(N);
    return output;
}

/**
 * @brief 重置所有统计信息
 */
void StereoWidener4::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
