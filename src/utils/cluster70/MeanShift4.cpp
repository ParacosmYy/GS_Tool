/**
 * @file MeanShift4.cpp
 * @brief MeanShift均值漂移聚类实现
 *
 * 实现基于核密度估计的MeanShift聚类算法，通过迭代
 * 将每个点移动到局部密度最大值（模式），自动确定簇数。
 */

#include "utils/cluster70/MeanShift4.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/**
 * @brief 构造函数，初始化默认参数
 * @param parent 父对象指针
 */
MeanShift4::MeanShift4(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置核带宽
 * @param bw 带宽参数
 */
void MeanShift4::setBandwidth(double bw)
{
    m_bandwidth = qBound(0.01, bw, 1000.0);
}

/**
 * @brief 设置核函数类型
 * @param type 核函数："gaussian" 或 "flat"
 */
void MeanShift4::setKernelType(const QString& type)
{
    if (type == "gaussian" || type == "flat") {
        m_kernel = type;
    }
}

/**
 * @brief 设置最大迭代次数
 * @param iter 最大迭代次数
 */
void MeanShift4::setMaxIterations(int iter)
{
    m_maxIter = qMax(10, iter);
}

/**
 * @brief 对数据点进行MeanShift聚类
 * @param points 输入数据点集合
 * @return 每个点的聚类标签
 */
QVector<int> MeanShift4::cluster(const QVector<QVector<double>>& points)
{
    QElapsedTimer timer;
    timer.start();

    const int N = points.size();
    if (N == 0) return QVector<int>();

    const int D = points[0].size();

    // 对每个点执行MeanShift迭代
    QVector<QVector<double>> shifted(N);
    int totalIters = 0;

    for (int i = 0; i < N; ++i) {
        QVector<double> current = points[i];
        for (int iter = 0; iter < m_maxIter; ++iter) {
            QVector<double> next = shiftPoint(current, points);
            // 检查收敛
            double move = 0.0;
            for (int d = 0; d < D; ++d) {
                double diff = next[d] - current[d];
                move += diff * diff;
            }
            current = next;
            totalIters++;
            if (qSqrt(move) < 1e-6 * m_bandwidth) break;
        }
        shifted[i] = current;
    }

    // 聚合收敛点（距离小于带宽的合并为一个簇）
    m_modes.clear();
    QVector<int> labels(N, -1);

    for (int i = 0; i < N; ++i) {
        bool found = false;
        for (int c = 0; c < m_modes.size(); ++c) {
            double dist = 0.0;
            for (int d = 0; d < D; ++d) {
                double diff = shifted[i][d] - m_modes[c][d];
                dist += diff * diff;
            }
            if (qSqrt(dist) < m_bandwidth * 0.5) {
                labels[i] = c;
                found = true;
                break;
            }
        }
        if (!found) {
            labels[i] = m_modes.size();
            m_modes.append(shifted[i]);
        }
    }

    // 更新统计信息
    qint64 elapsed = timer.elapsed();
    m_stats.totalShifts++;
    m_stats.totalPoints += N;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalShifts;

    emit clusteringCompleted(m_modes.size(), totalIters / N);
    return labels;
}

/**
 * @brief 重置统计信息
 */
void MeanShift4::resetStatistics()
{
    m_stats = Stats();
    m_timeSum = 0.0;
}

/**
 * @brief 计算单个点的漂移目标
 * @param pt 当前点
 * @param all 所有点
 * @return 漂移后的新位置
 */
QVector<double> MeanShift4::shiftPoint(const QVector<double>& pt,
                                        const QVector<QVector<double>>& all)
{
    int D = pt.size();
    QVector<double> numerator(D, 0.0);
    double denominator = 0.0;

    for (const auto& other : all) {
        double distSq = 0.0;
        for (int d = 0; d < D; ++d) {
            double diff = pt[d] - other[d];
            distSq += diff * diff;
        }
        double w = kernelWeight(qSqrt(distSq));
        if (w > 0.0) {
            for (int d = 0; d < D; ++d) {
                numerator[d] += w * other[d];
            }
            denominator += w;
        }
    }

    QVector<double> result(D, 0.0);
    if (denominator > 1e-300) {
        for (int d = 0; d < D; ++d) {
            result[d] = numerator[d] / denominator;
        }
    }
    return result;
}

/**
 * @brief 核函数权重计算
 * @param dist 距离
 * @return 核权重值
 */
double MeanShift4::kernelWeight(double dist) const
{
    double ratio = dist / m_bandwidth;
    if (m_kernel == "gaussian") {
        return qExp(-0.5 * ratio * ratio);
    } else {
        // Flat (均匀)核
        return (ratio <= 1.0) ? 1.0 : 0.0;
    }
}
