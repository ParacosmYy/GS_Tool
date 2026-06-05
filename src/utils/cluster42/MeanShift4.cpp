/**
 * @file MeanShift4.cpp
 * @brief 均值漂移4实现 — 自适应带宽+均值漂移分割
 *
 * 均值漂移聚类算法:
 * - 对每个数据点执行均值漂移迭代直至收敛
 * - 收敛点即为聚类中心(模式)
 * - 支持多种核函数(Gaussian/Epanechnikov/Uniform)
 * - 自适应带宽参数
 *
 * 统计信息跟踪: 聚类次数、处理点数、漂移迭代次数、平均耗时。
 */

#include "utils/cluster42/MeanShift4.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <cmath>

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
 * @param bandwidth 核函数带宽(必须>0)
 */
void MeanShift4::setBandwidth(double bandwidth)
{
    m_bandwidth = qMax(1e-10, bandwidth);
}

/**
 * @brief 设置核函数类型
 * @param kernel 核函数名称: "gaussian"/"epanechnikov"/"uniform"
 */
void MeanShift4::setKernel(const QString& kernel)
{
    m_kernel = kernel.toLower();
}

/**
 * @brief 设置最大迭代次数
 * @param maxIter 最大迭代次数
 */
void MeanShift4::setMaxIterations(int maxIter)
{
    m_maxIterations = qMax(1, maxIter);
}

/**
 * @brief 设置收敛阈值
 * @param tol 收敛阈值(漂移距离小于此值时停止)
 */
void MeanShift4::setConvergenceThreshold(double tol)
{
    m_tol = qMax(1e-10, tol);
}

/**
 * @brief 计算核函数权重
 *
 * 高斯核: K(x) = exp(-||x||^2 / (2*h^2))
 * Epanechnikov核: K(x) = max(0, 1 - ||x||^2/h^2)
 * 均匀核: K(x) = 1 if ||x|| < h, else 0
 *
 * @param x 数据点
 * @param center 核中心
 * @return 核函数权重值
 */
double MeanShift4::kernelWeight(const QVector<double>& x,
                                 const QVector<double>& center) const
{
    // 计算平方距离
    double distSq = 0.0;
    int dim = qMin(x.size(), center.size());
    for (int i = 0; i < dim; ++i) {
        double diff = x[i] - center[i];
        distSq += diff * diff;
    }

    double hSq = m_bandwidth * m_bandwidth;

    if (m_kernel == "gaussian") {
        return qExp(-distSq / (2.0 * hSq));
    } else if (m_kernel == "epanechnikov") {
        double u = distSq / hSq;
        return (u <= 1.0) ? (1.0 - u) : 0.0;
    } else {
        // 均匀核
        return (distSq <= hSq) ? 1.0 : 0.0;
    }
}

/**
 * @brief 对单个点执行均值漂移
 *
 * Mean Shift迭代公式:
 * x_new = Σ K(x_i - x) * x_i / Σ K(x_i - x)
 *
 * @param point 当前点
 * @param data 全部数据
 * @return 漂移后的新位置
 */
QVector<double> MeanShift4::shiftPoint(const QVector<double>& point,
                                        const QVector<QVector<double>>& data) const
{
    int dim = point.size();
    QVector<double> shifted(dim, 0.0);
    double totalWeight = 0.0;

    for (const auto& xi : data) {
        double w = kernelWeight(xi, point);
        if (w > 0.0) {
            for (int d = 0; d < dim && d < xi.size(); ++d) {
                shifted[d] += w * xi[d];
            }
            totalWeight += w;
        }
    }

    if (totalWeight > 1e-15) {
        for (double& v : shifted) {
            v /= totalWeight;
        }
    }

    return shifted;
}

/**
 * @brief 对数据进行均值漂移聚类
 *
 * 执行流程:
 * 1. 对每个数据点执行均值漂移迭代直至收敛
 * 2. 收集所有收敛点(模式)
 * 3. 合并距离小于带宽的相近模式
 * 4. 分配每个点到最近的模式
 *
 * @param data 输入数据，每个元素是一个特征向量
 * @return 每个数据点的聚类标签
 */
QVector<int> MeanShift4::fit(const QVector<QVector<double>>& data)
{
    QElapsedTimer timer;
    timer.start();

    const int n = data.size();
    if (n == 0) {
        return {};
    }

    int totalIters = 0;

    // 步骤1: 对每个点执行均值漂移
    QVector<QVector<double>> modes(n);
    for (int i = 0; i < n; ++i) {
        QVector<double> current = data[i];
        for (int iter = 0; iter < m_maxIterations; ++iter) {
            QVector<double> shifted = shiftPoint(current, data);

            // 计算漂移距离
            double dist = 0.0;
            for (int d = 0; d < current.size() && d < shifted.size(); ++d) {
                double diff = current[d] - shifted[d];
                dist += diff * diff;
            }
            dist = qSqrt(dist);

            current = shifted;
            totalIters++;

            if (dist < m_tol) break;
        }
        modes[i] = current;
    }

    // 步骤2: 合并相近的模式
    QVector<QVector<double>> uniqueModes;
    QVector<int> modeLabels(n, -1);

    for (int i = 0; i < n; ++i) {
        bool merged = false;
        for (int j = 0; j < uniqueModes.size(); ++j) {
            double dist = 0.0;
            int dim = qMin(modes[i].size(), uniqueModes[j].size());
            for (int d = 0; d < dim; ++d) {
                double diff = modes[i][d] - uniqueModes[j][d];
                dist += diff * diff;
            }
            dist = qSqrt(dist);

            if (dist < m_bandwidth * 0.5) {
                modeLabels[i] = j;
                merged = true;
                break;
            }
        }

        if (!merged) {
            modeLabels[i] = uniqueModes.size();
            uniqueModes.append(modes[i]);
        }
    }

    // 步骤3: 存储模式中心
    m_modes = uniqueModes;

    // 更新统计信息
    m_stats.totalClusterings++;
    m_stats.totalPointsProcessed += n;
    m_stats.totalShiftIterations += totalIters;

    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalClusterings;

    emit clusteringCompleted(n, uniqueModes.size());
    return modeLabels;
}

/**
 * @brief 重置所有统计信息
 */
void MeanShift4::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
