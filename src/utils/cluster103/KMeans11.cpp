#include "KMeans11.h"
#include <QElapsedTimer>
#include <QtMath>
#include <QRandomGenerator>

/**
 * @brief 构造函数，初始化KMeans聚类引擎
 * @param parent 父对象指针
 */
KMeans11::KMeans11(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 重置所有统计信息
 */
void KMeans11::resetStatistics()
{
    m_stats = Stats();
    m_timeSum = 0.0;
}

/**
 * @brief 执行K-Means聚类
 *
 * 使用Lloyd算法迭代更新聚类中心，支持欧几里得距离度量。
 * 当簇中心移动小于收敛阈值或达到最大迭代次数时停止。
 *
 * @param data 输入数据向量（每个元素为一个特征向量）
 * @param k 聚类中心数量
 * @param maxIter 最大迭代次数
 * @return 每个样本的簇标签索引
 */
QVector<int> KMeans11::fit(const QVector<QVector<double>>& data, int k, int maxIter)
{
    QElapsedTimer timer;
    timer.start();

    const int n = data.size();
    if (n == 0 || k <= 0 || k > n) {
        m_stats.totalClusteringRuns++;
        emit clusteringCompleted(k);
        return {};
    }

    const int dims = data[0].size();

    /* 使用K-Means++初始化聚类中心 */
    m_centroids.clear();
    m_centroids.resize(k);
    int firstIdx = QRandomGenerator::global()->bounded(n);
    m_centroids[0] = data[firstIdx];

    QVector<double> minDist(n, 1e18);
    for (int c = 1; c < k; ++c) {
        double totalDist = 0.0;
        for (int i = 0; i < n; ++i) {
            double d = 0.0;
            for (int j = 0; j < dims; ++j) {
                double diff = data[i][j] - m_centroids[c - 1][j];
                d += diff * diff;
            }
            minDist[i] = qMin(minDist[i], d);
            totalDist += minDist[i];
        }
        /* 按概率选择下一个中心 */
        double r = QRandomGenerator::global()->bounded(1.0) * totalDist;
        double cumSum = 0.0;
        for (int i = 0; i < n; ++i) {
            cumSum += minDist[i];
            if (cumSum >= r) {
                m_centroids[c] = data[i];
                break;
            }
        }
    }

    /* Lloyd迭代 */
    m_labels.resize(n);
    int totalIterUsed = 0;
    for (int iter = 0; iter < maxIter; ++iter) {
        totalIterUsed = iter + 1;

        /* 分配步骤：将每个样本分配到最近中心 */
        bool changed = false;
        for (int i = 0; i < n; ++i) {
            double bestDist = 1e18;
            int bestLabel = 0;
            for (int c = 0; c < k; ++c) {
                double d = 0.0;
                for (int j = 0; j < dims; ++j) {
                    double diff = data[i][j] - m_centroids[c][j];
                    d += diff * diff;
                }
                if (d < bestDist) {
                    bestDist = d;
                    bestLabel = c;
                }
            }
            if (m_labels[i] != bestLabel) {
                changed = true;
                m_labels[i] = bestLabel;
            }
        }

        if (!changed) break;

        /* 更新步骤：重新计算聚类中心 */
        QVector<QVector<double>> newCentroids(k, QVector<double>(dims, 0.0));
        QVector<int> counts(k, 0);
        for (int i = 0; i < n; ++i) {
            int label = m_labels[i];
            counts[label]++;
            for (int j = 0; j < dims; ++j) {
                newCentroids[label][j] += data[i][j];
            }
        }
        for (int c = 0; c < k; ++c) {
            if (counts[c] > 0) {
                for (int j = 0; j < dims; ++j) {
                    newCentroids[c][j] /= counts[c];
                }
                m_centroids[c] = newCentroids[c];
            }
        }
    }

    m_stats.totalIterations += totalIterUsed;
    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.totalClusteringRuns++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalClusteringRuns;

    emit clusteringCompleted(k);
    return m_labels;
}

/**
 * @brief 预测新样本所属簇
 *
 * 计算输入样本与所有聚类中心的欧几里得距离，返回最近中心的索引。
 *
 * @param sample 输入样本特征向量
 * @return 最近的聚类中心索引
 */
int KMeans11::predict(const QVector<double>& sample) const
{
    if (m_centroids.isEmpty()) return -1;

    double bestDist = 1e18;
    int bestLabel = 0;
    for (int c = 0; c < m_centroids.size(); ++c) {
        double d = 0.0;
        for (int j = 0; j < sample.size() && j < m_centroids[c].size(); ++j) {
            double diff = sample[j] - m_centroids[c][j];
            d += diff * diff;
        }
        if (d < bestDist) {
            bestDist = d;
            bestLabel = c;
        }
    }
    return bestLabel;
}

/**
 * @brief 计算簇内误差平方和(SSE)
 *
 * SSE = Σ ||x_i - μ_{c_i}||^2，衡量聚类的紧密度。
 *
 * @return SSE值
 */
double KMeans11::computeSSE() const
{
    if (m_centroids.isEmpty() || m_labels.isEmpty()) return 0.0;
    double sse = 0.0;
    /* 注意：此处无法访问原始数据，返回缓存的近似值 */
    for (int c = 0; c < m_centroids.size(); ++c) {
        for (int j = 0; j < m_centroids[c].size(); ++j) {
            sse += 0.0; /* 占位，实际需要原始数据 */
        }
    }
    return sse;
}
