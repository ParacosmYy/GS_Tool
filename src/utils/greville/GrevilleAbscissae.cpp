/**
 * @file GrevilleAbscissae.cpp
 * @brief Greville横坐标计算实现 — B样条节点向量平均坐标
 */

#include "utils/greville/GrevilleAbscissae.h"

#include <QElapsedTimer>
#include <cmath>

/** @brief 构造函数 @param parent 父对象 */
GrevilleAbscissae::GrevilleAbscissae(QObject* parent)
    : QObject(parent)
{
}

/** @brief 计算Greville横坐标 */
QVector<double> GrevilleAbscissae::compute(const QVector<double>& knots,
                                            int degree)
{
    QElapsedTimer timer;
    timer.start();

    /* 参数校验 */
    int m = knots.size();
    if (degree < 1 || m < degree + 2) {
        emit computationCompleted(0);
        return {};
    }

    /* Greville横坐标数量 = m - degree - 1 */
    int nPoints = m - degree - 1;
    QVector<double> abscissae(nPoints);

    for (int i = 0; i < nPoints; ++i) {
        double sum = 0.0;
        for (int j = 1; j <= degree; ++j)
            sum += knots[i + j];
        abscissae[i] = sum / static_cast<double>(degree);
    }

    /* 更新统计 */
    m_stats.totalComputations++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum /
        static_cast<double>(m_stats.totalComputations);

    emit computationCompleted(nPoints);
    return abscissae;
}

/** @brief 重置统计 */
void GrevilleAbscissae::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
