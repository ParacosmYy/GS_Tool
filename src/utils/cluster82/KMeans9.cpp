#include "KMeans9.h"
#include <QElapsedTimer>
#include <cmath>
#include <algorithm>
#include <random>

/**
 * @brief 构造函数，初始化K-Means聚类器
 * @param parent 父QObject对象指针
 */
KMeans9::KMeans9(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 拟合K-Means模型
 *
 * K-Means++初始化 + Lloyd迭代：
 * 1. K-Means++选择初始质心（概率与距离成正比）
 * 2. 将每个样本分配到最近的质心
 * 3. 重新计算每个簇的质心（均值）
 * 4. 重复2-3直到收敛或达到最大迭代次数
 *
 * @param data 输入数据，每行为一个样本
 * @param k 簇数量
 * @param maxIter 最大迭代次数，默认300
 * @return true如果成功收敛，false如果数据不足
 */
bool KMeans9::fit(const QVector<QVector<double>>& data, int k, int maxIter)
{
    QElapsedTimer timer;
    timer.start();

    const int n = data.size();
    if (n < k || k <= 0 || n == 0) return false;

    const int dim = data[0].size();

    /// K-Means++初始化质心
    std::mt19937 rng(42);
    std::uniform_int_distribution<int> uniformDist(0, n - 1);

    m_centroids.clear();
    m_centroids.reserve(k);

    /// 选择第一个质心（随机）
    m_centroids.append(data[uniformDist(rng)]);

    /// 依次选择后续质心
    QVector<double> minDist(n, 1e30);
    for (int c = 1; c < k; ++c) {
        /// 计算每个点到最近质心的距离
        double totalDist = 0.0;
        for (int i = 0; i < n; ++i) {
            double d = euclideanDist(data[i], m_centroids.last());
            minDist[i] = qMin(minDist[i], d * d);
            totalDist += minDist[i];
        }

        /// 按距离概率选择下一个质心
        std::uniform_real_distribution<double> probDist(0.0, totalDist);
        double threshold = probDist(rng);
        double cumulative = 0.0;
        int selected = 0;
        for (int i = 0; i < n; ++i) {
            cumulative += minDist[i];
            if (cumulative >= threshold) { selected = i; break; }
        }
        m_centroids.append(data[selected]);
    }

    /// Lloyd迭代主循环
    for (int iter = 0; iter < maxIter; ++iter) {
        /// 分配步骤：将每个样本分配到最近质心
        QVector<int> assignments(n);
        for (int i = 0; i < n; ++i) {
            double minD = 1e30;
            for (int c = 0; c < k; ++c) {
                double d = euclideanDist(data[i], m_centroids[c]);
                if (d < minD) { minD = d; assignments[i] = c; }
            }
        }

        /// 更新步骤：重新计算质心
        QVector<QVector<double>> newCentroids(k, QVector<double>(dim, 0.0));
        QVector<int> clusterSizes(k, 0);

        for (int i = 0; i < n; ++i) {
            int c = assignments[i];
            ++clusterSizes[c];
            for (int d = 0; d < dim; ++d) {
                newCentroids[c][d] += data[i][d];
            }
        }

        for (int c = 0; c < k; ++c) {
            if (clusterSizes[c] > 0) {
                for (int d = 0; d < dim; ++d) {
                    newCentroids[c][d] /= clusterSizes[c];
                }
            }
        }

        /// 计算惯量（用于判断收敛）
        double inertia = 0.0;
        for (int i = 0; i < n; ++i) {
            inertia += euclideanDist(data[i], m_centroids[assignments[i]]);
        }

        /// 检查质心是否收敛
        double centroidShift = 0.0;
        for (int c = 0; c < k; ++c) {
            centroidShift += euclideanDist(m_centroids[c], newCentroids[c]);
        }
        m_centroids = newCentroids;

        emit iterationCompleted(iter + 1, inertia);

        if (centroidShift < 1e-6) break;  ///< 收敛
    }

    /// 更新统计信息
    m_stats.totalFitted++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalFitted;

    return true;
}

/**
 * @brief 预测样本所属簇
 *
 * 将每个样本分配到距离最近的质心对应的簇。
 * 需要先调用fit()训练模型。
 *
 * @param data 待预测的样本集合
 * @return 每个样本的簇编号(0~k-1)，未训练时返回空
 */
QVector<int> KMeans9::predict(const QVector<QVector<double>>& data) const
{
    const int n = data.size();
    if (m_centroids.isEmpty() || n == 0) return {};

    const int k = m_centroids.size();
    QVector<int> labels(n);

    for (int i = 0; i < n; ++i) {
        double minDist = 1e30;
        for (int c = 0; c < k; ++c) {
            double d = euclideanDist(data[i], m_centroids[c]);
            if (d < minDist) { minDist = d; labels[i] = c; }
        }
    }

    return labels;
}

/**
 * @brief 计算欧氏距离
 */
double KMeans9::euclideanDist(const QVector<double>& a, const QVector<double>& b) const
{
    double sum = 0.0;
    const int dim = qMin(a.size(), b.size());
    for (int i = 0; i < dim; ++i) {
        double diff = a[i] - b[i];
        sum += diff * diff;
    }
    return std::sqrt(sum);
}

/**
 * @brief 获取当前统计数据
 * @return 包含拟合次数、预测次数和平均耗时的Stats结构
 */
KMeans9::Stats KMeans9::stats() const
{
    return m_stats;
}

/**
 * @brief 重置所有统计数据为零值，清除质心
 */
void KMeans9::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_centroids.clear();
}
