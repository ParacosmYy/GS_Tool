/**
 * @file PhaseDetector.cpp
 * @brief 相位检测器实现
 */

#include "utils/phase/PhaseDetector.h"

#include <QElapsedTimer>
#include <QtMath>

/** @brief 构造函数 @param parent 父对象 */
PhaseDetector::PhaseDetector(QObject* parent)
    : QObject(parent)
    , m_timeSum(0.0)
{
}

/** @brief 提取瞬时相位 */
QVector<double> PhaseDetector::extractPhase(
    const QVector<double>& analytic) const
{
    int n = analytic.size() / 2;
    QVector<double> phase(n);
    for (int i = 0; i < n; ++i) {
        double re = analytic[2 * i];
        double im = analytic[2 * i + 1];
        phase[i] = qAtan2(im, re);
    }
    return phase;
}

/** @brief 相位解卷绕 */
QVector<double> PhaseDetector::unwrapPhase(
    const QVector<double>& phase) const
{
    int n = phase.size();
    if (n == 0) return phase;

    QVector<double> unwrapped(n);
    unwrapped[0] = phase[0];

    for (int i = 1; i < n; ++i) {
        double diff = phase[i] - unwrapped[i - 1];
        /* 归一化到[-pi, pi] */
        while (diff > M_PI) diff -= 2.0 * M_PI;
        while (diff < -M_PI) diff += 2.0 * M_PI;
        unwrapped[i] = unwrapped[i - 1] + diff;
    }
    return unwrapped;
}

/** @brief 瞬时频率 */
QVector<double> PhaseDetector::instantaneousFrequency(
    const QVector<double>& unwrappedPhase,
    double sampleRate) const
{
    int n = unwrappedPhase.size();
    if (n < 2) return QVector<double>();

    QVector<double> freq(n - 1);
    for (int i = 0; i < n - 1; ++i) {
        double dPhi = unwrappedPhase[i + 1] - unwrappedPhase[i];
        freq[i] = dPhi * sampleRate / (2.0 * M_PI);
    }
    return freq;
}

/** @brief 相位差 */
QVector<double> PhaseDetector::phaseDifference(
    const QVector<double>& phase1,
    const QVector<double>& phase2) const
{
    int n = qMin(phase1.size(), phase2.size());
    QVector<double> diff(n);
    for (int i = 0; i < n; ++i) {
        double d = phase1[i] - phase2[i];
        while (d > M_PI) d -= 2.0 * M_PI;
        while (d < -M_PI) d += 2.0 * M_PI;
        diff[i] = d;
    }
    return diff;
}

/** @brief 重置统计 */
void PhaseDetector::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
