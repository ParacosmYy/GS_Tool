#include "OPTICS9.h"
#include <QElapsedTimer>
#include <QMap>
#include <QtMath>
#include <algorithm>

/* ---- 内部状态存储 ---- */
static QVector<double> g_reachDist;
static QVector<double> g_coreDist;
static QVector<int> g_orderedIndices;

/**
 * @brief 构造函数，初始化OPTICS聚类引擎
 * @param parent 父对象指针
 */
OPTICS9::OPTICS9(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 重置所有统计信息
 */
void OPTICS9::resetStatistics()
{
    m_stats = Stats();
    m_timeSum = 0.0;
    g_reachDist.clear();
    g_coreDist.clear();
    g_orderedIndices.clear();
}

/**
 * @brief 执行OPTICS聚类分析
 *
 * 按可达距离递增顺序输出点序列，生成可达距离图。
 * 算法流程：
 * 1. 计算每个点的核心距离
 * 2. 从未处理点出发，按最小可达距离选择下一个处理点
 * 3. 更新邻域内点的可达距离
 *
 * @param dataPoints 输入数据点集合
 * @param epsilon 邻域半径
 * @param minPoints 核心点最小邻居数
 * @return 有序点索引序列
 */
QVector<int> OPTICS9::fit(const QVector<QVector<double>>& dataPoints,
                           double epsilon, int minPoints)
{
    QElapsedTimer timer;
    timer.start();

    const int n = dataPoints.size();
    g_orderedIndices.clear();
    g_reachDist = QVector<double>(n, 1e18);
    g_coreDist = QVector<double>(n, -1.0);

    if (n == 0) {
        emit clusteringCompleted(0);
        return g_orderedIndices;
    }

    /* 处理状态：0=未处理, 1=已处理 */
    QVector<int> processed(n, 0);

    for (int i = 0; i < n; ++i) {
        if (processed[i]) continue;

        /* 计算核心距离 */
        g_coreDist[i] = coreDistance(i, dataPoints, epsilon, minPoints);
        processed[i] = 1;
        g_orderedIndices.append(i);

        if (g_coreDist[i] < 0) continue; /* 非核心点 */

        /* 种子列表：待处理的邻域点及其可达距离 */
        QMap<int, double> seeds;
        /* 查找邻域 */
        const auto& pt = dataPoints[i];
        for (int j = 0; j < n; ++j) {
            if (j == i || processed[j]) continue;
            double dist = 0.0;
            const int dim = qMin(pt.size(), dataPoints[j].size());
            for (int d = 0; d < dim; ++d) {
                double diff = pt[d] - dataPoints[j][d];
                dist += diff * diff;
            }
            dist = qSqrt(dist);
            if (dist <= epsilon) {
                double newReach = qMax(g_coreDist[i], dist);
                if (!seeds.contains(j) || newReach < seeds[j]) {
                    seeds[j] = newReach;
                }
            }
        }

        /* 按最小可达距离依次处理种子点 */
        while (!seeds.isEmpty()) {
            /* 选择可达距离最小的点 */
            int bestPt = -1;
            double bestReach = 1e18;
            for (auto it = seeds.begin(); it != seeds.end(); ++it) {
                if (it.value() < bestReach) {
                    bestReach = it.value();
                    bestPt = it.key();
                }
            }
            if (bestPt < 0) break;

            seeds.remove(bestPt);
            g_reachDist[bestPt] = bestReach;
            g_coreDist[bestPt] = coreDistance(bestPt, dataPoints, epsilon, minPoints);
            processed[bestPt] = 1;
            g_orderedIndices.append(bestPt);

            /* 扩展：更新邻域内未处理点的可达距离 */
            if (g_coreDist[bestPt] >= 0) {
                const auto& bpt = dataPoints[bestPt];
                for (int j = 0; j < n; ++j) {
                    if (j == bestPt || processed[j]) continue;
                    double dist = 0.0;
                    const int dim = qMin(bpt.size(), dataPoints[j].size());
                    for (int d = 0; d < dim; ++d) {
                        double diff = bpt[d] - dataPoints[j][d];
                        dist += diff * diff;
                    }
                    dist = qSqrt(dist);
                    if (dist <= epsilon) {
                        double newReach = qMax(g_coreDist[bestPt], dist);
                        if (!seeds.contains(j) || newReach < seeds[j]) {
                            seeds[j] = newReach;
                        }
                    }
                }
            }
        }
    }

    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.totalClusterOps++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalClusterOps;

    emit clusteringCompleted(g_orderedIndices.size());
    return g_orderedIndices;
}

/**
 * @brief 获取可达距离序列
 *
 * 返回与有序点序列对应的可达距离值，
 * 第一个点的可达距离为无穷大（无前驱）。
 *
 * @return 各点的可达距离值
 */
QVector<double> OPTICS9::reachabilityDistances() const
{
    return g_reachDist;
}

/**
 * @brief 从可达距离图提取聚类（ξ陡度方法）
 *
 * 通过分析可达距离图的陡峭区间来识别聚类结构：
 * 1. 标记所有陡增点（可达距离急剧上升）
 * 2. 标记所有陡降点（可达距离急剧下降）
 * 3. 匹配陡增-陡降对形成聚类
 *
 * @param xi 深度阈值参数（0~1，越小越敏感）
 * @return 各点的聚类标签（-1表示噪声）
 */
QVector<int> OPTICS9::extractClusters(double xi)
{
    const int n = g_orderedIndices.size();
    QVector<int> labels(n, -1);
    if (n == 0) return labels;

    /* 构建有序可达距离序列 */
    QVector<double> orderedReach(n);
    for (int i = 0; i < n; ++i) {
        int idx = g_orderedIndices[i];
        orderedReach[i] = (idx >= 0 && idx < g_reachDist.size())
                           ? g_reachDist[idx] : 1e18;
    }

    /* 检测陡增区间（steep up）和陡降区间（steep down） */
    int clusterId = 0;
    int i = 0;
    while (i < n - 1) {
        /* 寻找陡降起点 */
        double ratioDown = (orderedReach[i] > 1e-12)
                           ? orderedReach[i + 1] / orderedReach[i] : 1.0;
        if (ratioDown < (1.0 - xi)) {
            /* 陡降开始，记录区间 */
            int startDown = i;
            while (i < n - 1) {
                double r = (orderedReach[i] > 1e-12)
                           ? orderedReach[i + 1] / orderedReach[i] : 1.0;
                if (r >= (1.0 - xi)) break;
                i++;
            }
            int endDown = i;

            /* 寻找对应的陡增区间 */
            int startUp = -1, endUp = -1;
            for (int j = endDown + 1; j < n - 1; ++j) {
                double r = (orderedReach[j] > 1e-12)
                           ? orderedReach[j + 1] / orderedReach[j] : 1.0;
                if (r > (1.0 + xi)) {
                    if (startUp < 0) startUp = j;
                    endUp = j + 1;
                }
            }

            /* 分配聚类标签 */
            int cStart = startDown;
            int cEnd = (endUp > startUp) ? endUp : endDown;
            cEnd = qMin(cEnd, n - 1);
            for (int k = cStart; k <= cEnd; ++k) {
                labels[k] = clusterId;
            }
            clusterId++;
        }
        i++;
    }

    return labels;
}

/**
 * @brief 计算核心距离
 *
 * 核心距离定义为使得点成为核心点的最小半径：
 * 即到第minPoints个最近邻的距离。
 * 若邻域点数不足minPoints，返回-1表示非核心点。
 *
 * @param pointIndex 点索引
 * @param dataPoints 数据点集合
 * @param epsilon 邻域半径
 * @param minPoints 最小邻居数
 * @return 核心距离值（-1表示非核心点）
 */
double OPTICS9::coreDistance(int pointIndex, const QVector<QVector<double>>& dataPoints,
                              double epsilon, int minPoints) const
{
    const int n = dataPoints.size();
    if (pointIndex < 0 || pointIndex >= n) return -1.0;

    /* 计算到所有邻域点的距离 */
    const auto& pt = dataPoints[pointIndex];
    const int dim = pt.size();
    QVector<double> dists;

    for (int j = 0; j < n; ++j) {
        if (j == pointIndex) continue;
        const int dMin = qMin(dim, dataPoints[j].size());
        double dist = 0.0;
        for (int d = 0; d < dMin; ++d) {
            double diff = pt[d] - dataPoints[j][d];
            dist += diff * diff;
        }
        dist = qSqrt(dist);
        if (dist <= epsilon) {
            dists.append(dist);
        }
    }

    /* 邻域点不足则非核心点 */
    if (dists.size() < static_cast<int>(minPoints)) return -1.0;

    /* 排序取第minPoints-1个距离 */
    std::sort(dists.begin(), dists.end());
    return dists[qMin(minPoints - 1, dists.size() - 1)];
}
