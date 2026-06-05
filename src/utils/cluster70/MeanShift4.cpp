/**
 * @file MeanShift4.cpp
 * @brief MeanShift均值漂移聚类算法实现
 *
 * 实现基于核密度估计的均值漂移聚类算法，通过迭代将每个点
 * 移向局部密度最大的位置(模态)，最终收敛到相同模态的点归为同一簇。
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
 * @brief 设置核函数带宽
 * @param bw 带宽参数，影响聚类粒度
 */
void MeanShift4::setBandwidth(double bw)
{
    m_bandwidth = qBound(0.01, bw, 1000.0);
}

/**
 * @brief 设置核函数类型
 * @param type 核函数名称: "gaussian" 或 "flat"
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
 * @brief 对数据点执行MeanShift聚类
 * @param points 输入数据点集合
 * @return 每个点的簇标签
 *
 * 算法流程:
 * 1. 对每个点迭代执行均值漂移直到收敛
 * 2. 收敛位置即为模态(聚类中心)
 * 3. 合并距离小于带宽的模态为同一簇
 */
QVector<int> MeanShift4::cluster(const QVector<QVector<double>>& points)
{
    QElapsedTimer timer;
    timer.start();

    const int N = points.size();
    QVector<int> labels(N, 0);
    m_modes.clear();

    if (N == 0) {
        m_timeSum += timer.elapsed();
        m_stats.totalShifts++;
        m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalShifts;
        return labels;
    }

    /* 阶段1: 对每个点执行均值漂移 */
    QVector<QVector<double>> converged(N);
    int totalIter = 0;

    for (int i = 0; i < N; ++i) {
        QVector<double> current = points[i];
        int iter = 0;

        while (iter < m_maxIter) {
            QVector<double> shifted = shiftPoint(current, points);

            /* 检查收敛: 位移小于阈值 */
            double moveDist = 0.0;
            for (int d = 0; d < shifted.size(); ++d) {
                double diff = shifted[d] - current[d];
                moveDist += diff * diff;
            }
            moveDist = qSqrt(moveDist);

            current = shifted;
            iter++;
            totalIter++;

            if (moveDist < 1e-6 * m_bandwidth) break;
        }

        converged[i] = current;
    }

    /* 阶段2: 合并相近的模态 */
    for (int i = 0; i < N; ++i) {
        int assignedCluster = -1;

        for (int c = 0; c < m_modes.size(); ++c) {
            double dist = 0.0;
            for (int d = 0; d < m_modes[c].size(); ++d) {
                double diff = converged[i][d] - m_modes[c][d];
                dist += diff * diff;
            }
            dist = qSqrt(dist);

            if (dist < m_bandwidth * 0.5) {
                assignedCluster = c;
                break;
            }
        }

        if (assignedCluster == -1) {
            assignedCluster = m_modes.size();
            m_modes.append(converged[i]);
        }

        labels[i] = assignedCluster;
    }

    /* 更新统计信息 */
    qint64 elapsed = timer.elapsed();
    m_stats.totalShifts++;
    m_stats.totalPoints += N;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalShifts;

    emit clusteringCompleted(m_modes.size(), totalIter);
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
 * @brief 对单个点执行一次均值漂移
 * @param pt 当前点位置
 * @param all 所有点集合
 * @return 漂移后的新位置
 *
 * 使用核函数加权的均值计算漂移方向:
 * new_x = sum(kernel(||x - xi||/h) * xi) / sum(kernel(||x - xi||/h))
 */
QVector<double> MeanShift4::shiftPoint(const QVector<double>& pt, const QVector<QVector<double>>& all)
{
    int D = pt.size();
    QVector<double> numerator(D, 0.0);
    double denominator = 0.0;

    for (const auto& xi : all) {
        double distSq = 0.0;
        for (int d = 0; d < D; ++d) {
            double diff = pt[d] - xi[d];
            distSq += diff * diff;
        }
        double dist = qSqrt(distSq);

        double w = kernelWeight(dist / m_bandwidth);
        if (w > 0.0) {
            for (int d = 0; d < D; ++d) {
                numerator[d] += w * xi[d];
            }
            denominator += w;
        }
    }

    QVector<double> result(D, 0.0);
    if (denominator > 1e-15) {
        for (int d = 0; d < D; ++d) {
            result[d] = numerator[d] / denominator;
        }
    } else {
        result = pt;
    }

    return result;
}

/**
 * @brief 计算核函数权重
 * @param dist 归一化距离 (distance / bandwidth)
 * @return 核函数权重值
 */
double MeanShift4::kernelWeight(double dist) const
{
    if (m_kernel == "flat") {
        /* Flat核: 带宽范围内权重为1 */
        return (dist <= 1.0) ? 1.0 : 0.0;
    } else {
        /* 高斯核: exp(-0.5 * dist^2) */
        return qExp(-0.5 * dist * dist);
    }
}
