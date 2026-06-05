/**
 * @file MeanShift3.cpp
 * @brief 均值漂移聚类实现 — 高斯核均值漂移 + 模式检测
 * @author EmbedDebug Team
 * @date 2026-06-05
 *
 * 实现基于高斯核的均值漂移（Mean Shift）聚类算法。
 * 每个数据点沿密度梯度方向迭代移动至最近的密度模式（局部最大值），
 * 收敛到同一模式的点被归为同一聚类。
 */

#include "utils/cluster54/MeanShift3.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

// ──────────────────────────────────────────────
// 构造函数
// ──────────────────────────────────────────────

/**
 * @brief 构造函数，初始化默认均值漂移参数
 * @param parent 父QObject对象
 */
MeanShift3::MeanShift3(QObject* parent)
    : QObject(parent)
{
    setObjectName(QStringLiteral("MeanShift3"));
}

// ──────────────────────────────────────────────
// 参数配置
// ──────────────────────────────────────────────

/**
 * @brief 设置核函数带宽
 *
 * 带宽决定了搜索窗口的大小。带宽越大，聚类越少越平滑；
 * 带宽越小，聚类越多越细致。
 *
 * @param bw 核函数带宽，必须 > 0
 */
void MeanShift3::setBandwidth(double bw)
{
    m_bandwidth = qMax(1e-6, bw);
}

/**
 * @brief 设置最大迭代次数
 *
 * 每个点的漂移迭代在达到最大次数后强制停止。
 *
 * @param iter 最大迭代次数
 */
void MeanShift3::setMaxIterations(int iter)
{
    m_maxIter = qMax(1, iter);
}

// ──────────────────────────────────────────────
// 核心聚类接口
// ──────────────────────────────────────────────

/**
 * @brief 对输入数据执行均值漂移聚类
 *
 * 处理流程：
 * 1. 对每个数据点执行均值漂移迭代，直到收敛
 * 2. 收集所有收敛点（模式）
 * 3. 合并距离小于带宽的模式
 * 4. 分配聚类标签
 *
 * @param points 输入数据点集合
 * @return 聚类标签数组，与输入点一一对应（0-based）
 */
QVector<int> MeanShift3::cluster(const QVector<QVector<double>>& points)
{
    QElapsedTimer timer;
    timer.start();

    const int n = points.size();
    if (n == 0) {
        m_modes.clear();
        return {};
    }

    // 步骤1：对每个点执行均值漂移
    int totalIterations = 0;
    QVector<QVector<double>> shiftedPoints(n);

    for (int i = 0; i < n; ++i) {
        QVector<double> current = points[i];
        for (int iter = 0; iter < m_maxIter; ++iter) {
            QVector<double> shifted = shiftPoint(current, points);
            totalIterations++;

            // 检查收敛
            double dist = 0.0;
            const int dim = qMin(current.size(), shifted.size());
            for (int d = 0; d < dim; ++d) {
                double diff = current[d] - shifted[d];
                dist += diff * diff;
            }
            dist = qSqrt(dist);

            current = shifted;
            if (dist < 1e-6 * m_bandwidth) {
                break;
            }
        }
        shiftedPoints[i] = current;
    }

    // 步骤2：合并相近的模式
    m_modes.clear();
    QVector<int> labels(n, -1);

    for (int i = 0; i < n; ++i) {
        // 查找是否已有相近的模式
        int matchedMode = -1;
        for (int m = 0; m < m_modes.size(); ++m) {
            double dist = 0.0;
            const int dim = qMin(m_modes[m].size(), shiftedPoints[i].size());
            for (int d = 0; d < dim; ++d) {
                double diff = m_modes[m][d] - shiftedPoints[i][d];
                dist += diff * diff;
            }
            if (qSqrt(dist) < m_bandwidth * 0.5) {
                matchedMode = m;
                break;
            }
        }

        if (matchedMode < 0) {
            // 创建新模式
            matchedMode = m_modes.size();
            m_modes.append(shiftedPoints[i]);
        }
        labels[i] = matchedMode;
    }

    // 步骤3：合并模式的平均位置
    QVector<int> modeCounts(m_modes.size(), 0);
    QVector<QVector<double>> modeSums(m_modes.size(), QVector<double>());
    for (int i = 0; i < n; ++i) {
        int lbl = labels[i];
        modeCounts[lbl]++;
        const int dim = shiftedPoints[i].size();
        if (modeSums[lbl].isEmpty()) {
            modeSums[lbl].resize(dim, 0.0);
        }
        for (int d = 0; d < dim; ++d) {
            modeSums[lbl][d] += shiftedPoints[i][d];
        }
    }
    for (int m = 0; m < m_modes.size(); ++m) {
        if (modeCounts[m] > 0) {
            for (int d = 0; d < m_modes[m].size(); ++d) {
                m_modes[m][d] = modeSums[m][d] / modeCounts[m];
            }
        }
    }

    // 更新统计
    const double elapsed = timer.elapsed();
    m_stats.totalShifts++;
    m_stats.totalPoints += n;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalShifts;

    emit clusteringCompleted(m_modes.size(), totalIterations);
    return labels;
}

// ──────────────────────────────────────────────
// 统计接口
// ──────────────────────────────────────────────

/**
 * @brief 获取当前统计信息
 * @return 包含聚类次数、总点数和平均耗时的Stats结构
 */
MeanShift3::Stats MeanShift3::stats() const
{
    return m_stats;
}

/**
 * @brief 重置所有累计统计信息
 */
void MeanShift3::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

// ──────────────────────────────────────────────
// 私有方法 — 均值漂移迭代
// ──────────────────────────────────────────────

/**
 * @brief 对单个点执行一次均值漂移
 *
 * 使用高斯核计算所有点对当前点的加权平均：
 * shifted = sum(K(x-xi) * xi) / sum(K(x-xi))
 *
 * @param pt 当前点
 * @param all 所有数据点
 * @return 漂移后的新位置
 */
QVector<double> MeanShift3::shiftPoint(const QVector<double>& pt,
                                        const QVector<QVector<double>>& all)
{
    const int dim = pt.size();
    QVector<double> numerator(dim, 0.0);
    double denominator = 0.0;

    for (const auto& xi : all) {
        double dist = 0.0;
        const int d = qMin(dim, xi.size());
        for (int i = 0; i < d; ++i) {
            double diff = pt[i] - xi[i];
            dist += diff * diff;
        }
        dist = qSqrt(dist);

        double weight = gaussianKernel(dist);
        for (int i = 0; i < qMin(dim, xi.size()); ++i) {
            numerator[i] += weight * xi[i];
        }
        denominator += weight;
    }

    if (denominator < 1e-15) {
        return pt;
    }

    QVector<double> result(dim, 0.0);
    for (int i = 0; i < dim; ++i) {
        result[i] = numerator[i] / denominator;
    }
    return result;
}

// ──────────────────────────────────────────────
// 私有方法 — 高斯核函数
// ──────────────────────────────────────────────

/**
 * @brief 计算高斯核函数值
 *
 * K(d) = exp(-d^2 / (2 * h^2))
 *
 * @param dist 距离值
 * @return 核函数值，范围 [0, 1]
 */
double MeanShift3::gaussianKernel(double dist) const
{
    return qExp(-(dist * dist) / (2.0 * m_bandwidth * m_bandwidth));
}
