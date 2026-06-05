#include "MeanShift6.h"
#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/**
 * @class MeanShift6
 * @brief Mean Shift均值漂移聚类实现
 *
 * Mean Shift是一种基于核密度估计的非参数聚类算法。
 * 每个数据点沿密度梯度方向移动，最终收敛到密度函数的局部极大值(模式点)。
 * 同一模式点吸引的所有点归为一个簇，无需预先指定簇数。
 *
 * 核函数使用高斯核: K(x) = exp(-||x||^2 / (2*h^2))
 * 带宽h控制核的影响范围，决定了聚类的粒度。
 */

/**
 * @brief 构造函数
 * @param parent 父QObject
 */
MeanShift6::MeanShift6(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 执行Mean Shift聚类
 *
 * 对每个数据点执行均值漂移迭代:
 * 1. 计算当前点在高斯核加权下所有邻域点的均值
 * 2. 将当前点移动到均值位置
 * 3. 重复直到移动距离小于阈值或达到最大迭代次数
 * 4. 合并距离小于带宽的模式点
 * 5. 根据最近模式点分配簇标签
 *
 * @param data 输入数据集
 * @param bandwidth 核函数带宽(邻域半径)
 * @param maxIter 每个点的最大迭代次数
 * @return 每个数据点的簇标签向量
 */
QVector<int> MeanShift6::fit(const QVector<QVector<double>>& data, double bandwidth, int maxIter)
{
    QElapsedTimer timer;
    timer.start();

    m_modes.clear();
    QVector<int> labels(data.size(), -1);

    if (data.isEmpty() || bandwidth <= 0.0) {
        m_timeSum += timer.elapsed();
        return labels;
    }

    double bwSq = bandwidth * bandwidth;
    int dims = data[0].size();

    /* 对每个数据点执行均值漂移 */
    QVector<QVector<double>> shiftedPoints = data;

    for (int p = 0; p < data.size(); ++p) {
        QVector<double> current = data[p];

        for (int iter = 0; iter < maxIter; ++iter) {
            QVector<double> meanShift(dims, 0.0);
            double totalWeight = 0.0;

            /* 计算高斯核加权均值 */
            for (int i = 0; i < data.size(); ++i) {
                double distSq = 0.0;
                for (int d = 0; d < dims; ++d) {
                    double diff = current[d] - data[i][d];
                    distSq += diff * diff;
                }

                double weight = qExp(-distSq / (2.0 * bwSq));
                for (int d = 0; d < dims; ++d) {
                    meanShift[d] += weight * data[i][d];
                }
                totalWeight += weight;
            }

            if (totalWeight < 1e-15) break;

            /* 计算新的位置 */
            QVector<double> newPos(dims);
            double shiftDist = 0.0;
            for (int d = 0; d < dims; ++d) {
                newPos[d] = meanShift[d] / totalWeight;
                double diff = newPos[d] - current[d];
                shiftDist += diff * diff;
            }
            shiftDist = qSqrt(shiftDist);

            current = newPos;
            m_stats.totalPointsShifted++;

            if (shiftDist < bandwidth * 0.01) break;
        }

        shiftedPoints[p] = current;
    }

    /* 合并相近的模式点 */
    double mergeThreshold = bandwidth * 0.5;
    for (int p = 0; p < shiftedPoints.size(); ++p) {
        bool merged = false;
        for (int m = 0; m < m_modes.size(); ++m) {
            double dist = 0.0;
            for (int d = 0; d < dims; ++d) {
                double diff = shiftedPoints[p][d] - m_modes[m][d];
                dist += diff * diff;
            }
            if (qSqrt(dist) < mergeThreshold) {
                labels[p] = m;
                merged = true;
                break;
            }
        }
        if (!merged) {
            m_modes.append(shiftedPoints[p]);
            labels[p] = m_modes.size() - 1;
        }
    }

    m_stats.totalModesFound = m_modes.size();
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / qMax(1, m_stats.totalPointsShifted);

    emit shiftCompleted(0, 0.0);

    return labels;
}

/**
 * @brief 获取收敛的簇中心(模式点)
 *
 * 返回所有经过合并后的模式点列表。
 * 必须在fit()调用之后使用。
 *
 * @return 模式点列表，每个元素为一个坐标向量
 */
QVector<QVector<double>> MeanShift6::modes() const
{
    return m_modes;
}

/**
 * @brief 重置所有统计数据
 *
 * 将漂移计数、模式点计数和计时归零。
 */
void MeanShift6::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_modes.clear();
}
