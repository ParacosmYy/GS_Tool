/**
 * @file StereoEnhancer.cpp
 * @brief StereoEnhancer 实现
 *
 * 实现M/S立体声处理：LR↔MS编解码、宽度控制、
 * 中心/侧链增益调节和相位校正。
 */

#include "utils/dsp162/StereoEnhancer.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/**
 * @brief 构造函数
 * @param parent 父QObject
 */
StereoEnhancer::StereoEnhancer(QObject* parent)
    : QObject(parent)
{
}

void StereoEnhancer::setWidth(double width)
{
    m_width = qBound(0.0, width, 3.0);
}

void StereoEnhancer::setCenterLevel(double level)
{
    m_centerLevel = qBound(0.0, level, 2.0);
}

void StereoEnhancer::setSideLevel(double level)
{
    m_sideLevel = qBound(0.0, level, 2.0);
}

void StereoEnhancer::setPhaseCorrection(bool enable)
{
    m_phaseCorrection = enable;
}

/**
 * @brief 相位校正
 *
 * 检测M/S域中的反相成分，当mid为负时翻转side符号。
 */
double StereoEnhancer::correctPhase(double mid, double side) const
{
    if (!m_phaseCorrection) return side;

    /* 如果mid信号为负且side很大，可能是反相 */
    if (mid < 0.0 && qAbs(side) > qAbs(mid)) {
        return -side;
    }
    return side;
}

/**
 * @brief 处理立体声帧
 *
 * 1) LR -> MS编码: mid = (L+R)/2, side = (L-R)/2
 * 2) 应用宽度和增益: mid *= centerLevel, side *= width * sideLevel
 * 3) 可选相位校正
 * 4) MS -> LR解码: L = mid + side, R = mid - side
 */
QPair<QVector<double>, QVector<double>> StereoEnhancer::process(
    const QVector<double>& left, const QVector<double>& right)
{
    QElapsedTimer timer;
    timer.start();

    const int frames = qMin(left.size(), right.size());
    if (frames == 0) return {QVector<double>(), QVector<double>()};

    QVector<double> outLeft(frames);
    QVector<double> outRight(frames);

    double peakMid = 0.0;
    double peakSide = 0.0;

    for (int i = 0; i < frames; ++i) {
        /* LR -> MS */
        double mid = (left[i] + right[i]) * 0.5;
        double side = (left[i] - right[i]) * 0.5;

        /* 相位校正 */
        side = correctPhase(mid, side);

        /* 应用增益 */
        mid *= m_centerLevel;
        side *= m_width * m_sideLevel;

        /* 统计峰值 */
        double absMid = qAbs(mid);
        double absSide = qAbs(side);
        if (absMid > peakMid) peakMid = absMid;
        if (absSide > peakSide) peakSide = absSide;

        /* MS -> LR */
        outLeft[i] = mid + side;
        outRight[i] = mid - side;
    }

    m_stats.totalFrames++;
    m_stats.totalSamples += frames;
    m_stats.peakMid = qMax(m_stats.peakMid, peakMid);
    m_stats.peakSide = qMax(m_stats.peakSide, peakSide);
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = (m_stats.totalFrames > 0)
        ? m_timeSum / m_stats.totalFrames : 0.0;

    emit processCompleted(frames, peakMid);
    return {outLeft, outRight};
}

/**
 * @brief 提取Mid信号
 */
QVector<double> StereoEnhancer::extractMid(const QVector<double>& left,
                                             const QVector<double>& right) const
{
    const int n = qMin(left.size(), right.size());
    QVector<double> mid(n);
    for (int i = 0; i < n; ++i) {
        mid[i] = (left[i] + right[i]) * 0.5;
    }
    return mid;
}

/**
 * @brief 提取Side信号
 */
QVector<double> StereoEnhancer::extractSide(const QVector<double>& left,
                                              const QVector<double>& right) const
{
    const int n = qMin(left.size(), right.size());
    QVector<double> side(n);
    for (int i = 0; i < n; ++i) {
        side[i] = (left[i] - right[i]) * 0.5;
    }
    return side;
}

void StereoEnhancer::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
