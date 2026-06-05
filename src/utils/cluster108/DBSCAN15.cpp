#include "DBSCAN15.h"
#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/**
 * @brief 构造函数，初始化DBSCAN聚类引擎
 * @param parent 父对象指针
 */
DBSCAN15::DBSCAN15(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 重置所有统计信息
 */
void DBSCAN15::resetStatistics()
{
    m_stats = Stats();
    m_timeSum = 0.0;
}

/**
 * @brief 设置邻域半径参数
 *
 * epsilon定义了核心点的邻域范围，值越大簇越容易合并。
 *
 * @param epsilon 邻域半径（最小0.001）
 */
void DBSCAN15::setEpsilon(double epsilon)
{
    m_epsilon = qMax(0.001, epsilon);
}

/**
 * @brief 设置最小邻域点数
 *
 * 一个点在epsilon邻域内至少有MinPts个邻居才被标记为核心点。
 *
 * @param minPts 最小邻域点数（最小2）
 */
void DBSCAN15::setMinPts(int minPts)
{
    m_minPts = qMax(2, minPts);
}

/**
 * @brief 启用或禁用自适应邻域模式
 *
 * 启用后每个点的epsilon根据其k近邻距离自动调整，
 * 适用于不同密度簇共存的场景。
 *
 * @param enabled 是否启用自适应
 */
void DBSCAN15::setAdaptiveEpsilon(bool enabled)
{
    m_adaptive = enabled;
}

/**
 * @brief 计算两个向量的欧氏距离
 * @param a 向量a
 * @param b 向量b
 * @return 欧氏距离
 */
static double euclideanDist(const QVector<double>& a, const QVector<double>& b)
{
    double sum = 0.0;
    const int dim = qMin(a.size(), b.size());
    for (int d = 0; d < dim; ++d) {
        double diff = a[d] - b[d];
        sum += diff * diff;
    }
    return qSqrt(sum);
}

/**
 * @brief 计算每个点的k近邻距离
 *
 * 对每个点，计算到所有其他点的距离并排序，
 * 取第k小的距离作为局部密度估计。
 *
 * @param data 数据集
 * @param k k值（通常等于MinPts）
 * @return 每个点的第k近邻距离
 */
static QVector<double> computeKNNDistances(const QVector<QVector<double>>& data, int k)
{
    const int n = data.size();
    QVector<double> kDist(n, 0.0);

    for (int i = 0; i < n; ++i) {
        QVector<double> dists;
        dists.reserve(n - 1);
        for (int j = 0; j < n; ++j) {
            if (i != j) dists.append(euclideanDist(data[i], data[j]));
        }
        std::sort(dists.begin(), dists.end());
        int idx = qMin(k - 1, static_cast<int>(dists.size()) - 1);
        kDist[i] = (idx >= 0) ? dists[idx] : 0.0;
    }
    return kDist;
}

/**
 * @brief 执行DBSCAN聚类
 *
 * 经典DBSCAN流程，可选自适应邻域模式：
 * 1. 预计算距离矩阵
 * 2. 自适应模式下计算每个点的局部epsilon
 * 3. 遍历未标记点，通过邻域查询判断核心点
 * 4. 从核心点BFS扩展簇，合并密度可达的点
 * 5. 标记无法归入任何簇的点为噪声(label=-1)
 *
 * @param data 输入数据矩阵(n×dim)
 * @return 每个点的簇标签（-1为噪声点）
 */
QVector<int> DBSCAN15::fit(const QVector<QVector<double>>& data)
{
    QElapsedTimer timer;
    timer.start();

    const int n = data.size();
    QVector<int> labels(n, -1);

    if (n == 0) {
        emit clusteringCompleted(0, 0);
        return labels;
    }

    if (n == 1) {
        labels[0] = 0;
        emit clusteringCompleted(1, 0);
        return labels;
    }

    /* 预计算距离矩阵 */
    QVector<QVector<double>> dists(n, QVector<double>(n, 0.0));
    for (int i = 0; i < n; ++i) {
        for (int j = i + 1; j < n; ++j) {
            dists[i][j] = dists[j][i] = euclideanDist(data[i], data[j]);
        }
    }

    /* 自适应epsilon: 基于k近邻距离调整局部半径 */
    QVector<double> localEps(n, m_epsilon);
    if (m_adaptive) {
        QVector<double> kDists = computeKNNDistances(data, m_minPts);
        for (int i = 0; i < n; ++i) {
            localEps[i] = qMax(m_epsilon * 0.5, kDists[i] * 1.5);
        }
    }

    /* DBSCAN主循环 */
    int clusterId = 0;
    for (int i = 0; i < n; ++i) {
        if (labels[i] >= 0) continue;

        /* 查找epsilon邻域内的所有点 */
        QVector<int> neighbors;
        double eps = localEps[i];
        for (int j = 0; j < n; ++j) {
            if (dists[i][j] <= eps) neighbors.append(j);
        }

        /* 邻域内点数不足 → 标记为噪声 */
        if (neighbors.size() < m_minPts) {
            labels[i] = -1;
            continue;
        }

        /* 创建新簇，从当前点BFS扩展 */
        labels[i] = clusterId;
        QVector<int> seedQueue = neighbors;
        int qi = 0;
        while (qi < seedQueue.size()) {
            int cur = seedQueue[qi++];

            /* 噪声点可被后续核心点重新吸收 */
            if (labels[cur] == -1) {
                labels[cur] = clusterId;
            }
            /* 已分配到其他簇的点跳过 */
            if (labels[cur] >= 0 && cur != i) continue;

            labels[cur] = clusterId;

            /* 查找当前点的邻域 */
            double curEps = localEps[cur];
            QVector<int> curNeighbors;
            for (int j = 0; j < n; ++j) {
                if (dists[cur][j] <= curEps) curNeighbors.append(j);
            }

            /* 当前点也是核心点 → 扩展种子队列 */
            if (curNeighbors.size() >= m_minPts) {
                for (int nn : curNeighbors) {
                    if (labels[nn] < 0) {
                        /* 避免重复入队 */
                        bool inQueue = false;
                        for (int s = qi; s < seedQueue.size(); ++s) {
                            if (seedQueue[s] == nn) { inQueue = true; break; }
                        }
                        if (!inQueue) {
                            seedQueue.append(nn);
                        }
                    }
                }
            }
        }
        ++clusterId;
    }

    /* 统计噪声点数 */
    int noiseCount = 0;
    for (int i = 0; i < n; ++i) {
        if (labels[i] < 0) ++noiseCount;
    }

    /* 更新统计信息 */
    m_stats.totalClustered += n;
    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalClustered;

    emit clusteringCompleted(clusterId, noiseCount);
    return labels;
}
