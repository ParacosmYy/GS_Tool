#include "OPTICS8.h"
#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/**
 * @brief 构造函数，初始化OPTICS聚类引擎
 * @param parent 父对象指针
 */
OPTICS8::OPTICS8(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 重置所有统计信息
 */
void OPTICS8::resetStatistics()
{
    m_stats = Stats();
    m_timeSum = 0.0;
}

/**
 * @brief 设置核心距离邻域半径epsilon上限
 * @param epsilon 邻域半径值
 */
void OPTICS8::setEpsilon(double epsilon)
{
    m_epsilon = qMax(1e-6, epsilon);
}

/**
 * @brief 设置最小邻域点数MinPts，影响核心点判定
 * @param minPts 最小邻域点数
 */
void OPTICS8::setMinPts(int minPts)
{
    m_minPts = qMax(2, minPts);
}

/**
 * @brief 计算两点之间的欧氏距离
 * @param a 第一个点的特征向量
 * @param b 第二个点的特征向量
 * @return 欧氏距离
 */
static double euclideanDist(const QVector<double>& a, const QVector<double>& b)
{
    const int dims = qMin(a.size(), b.size());
    double sum = 0.0;
    for (int d = 0; d < dims; ++d) {
        double diff = a[d] - b[d];
        sum += diff * diff;
    }
    return qSqrt(sum);
}

/**
 * @brief 对输入数据执行OPTICS排序，生成可达距离有序列表
 *
 * 使用优先队列（种子列表）维护待处理点，每次取出可达距离最小的点。
 * 对核心点更新其邻域内点的可达距离，形成完整的排序序列。
 * 结果同时缓存到 m_orderingCache 供 extractClusters() 使用。
 *
 * @param data n×d数据矩阵（n个样本，d维特征）
 * @return (点索引, 可达距离)有序列表
 */
QVector<QPair<int, double>> OPTICS8::fit(const QVector<QVector<double>>& data)
{
    QElapsedTimer timer;
    timer.start();

    QVector<QPair<int, double>> ordering;
    const int n = data.size();
    if (n == 0) {
        emit orderingCompleted(0, 0.0);
        return ordering;
    }

    /* 预计算距离矩阵 */
    QVector<QVector<double>> dist(n, QVector<double>(n, 0.0));
    for (int i = 0; i < n; ++i) {
        for (int j = i + 1; j < n; ++j) {
            double d = euclideanDist(data[i], data[j]);
            dist[i][j] = d;
            dist[j][i] = d;
        }
    }

    /* 计算每个点的核心距离 */
    QVector<double> coreDist(n, 1e18);
    for (int i = 0; i < n; ++i) {
        QVector<double> neighbors;
        neighbors.reserve(n - 1);
        for (int j = 0; j < n; ++j) {
            if (j != i && dist[i][j] <= m_epsilon) {
                neighbors.append(dist[i][j]);
            }
        }
        if (neighbors.size() >= m_minPts - 1) {
            std::sort(neighbors.begin(), neighbors.end());
            coreDist[i] = neighbors[m_minPts - 2];
        }
    }

    QVector<bool> processed(n, false);
    QVector<double> reachDist(n, 1e18);
    double maxReach = 0.0;

    /* 逐点扩展OPTICS排序 */
    for (int start = 0; start < n; ++start) {
        if (processed[start]) continue;

        processed[start] = true;
        ordering.append({start, 0.0});

        /* 仅核心点才生成种子列表 */
        if (coreDist[start] < 1e18) {
            /* 种子列表：(可达距离, 点索引) */
            QVector<QPair<double, int>> seeds;

            for (int j = 0; j < n; ++j) {
                if (processed[j]) continue;
                double newRd = qMax(coreDist[start], dist[start][j]);
                if (newRd < reachDist[j]) {
                    reachDist[j] = newRd;
                    seeds.append({newRd, j});
                }
            }

            while (!seeds.isEmpty()) {
                /* 取可达距离最小的点 */
                int bestIdx = 0;
                for (int s = 1; s < seeds.size(); ++s) {
                    if (seeds[s].first < seeds[bestIdx].first) bestIdx = s;
                }
                int p = seeds[bestIdx].second;
                seeds.removeAt(bestIdx);

                if (processed[p]) continue;
                processed[p] = true;
                ordering.append({p, reachDist[p]});
                if (reachDist[p] < 1e17) maxReach = qMax(maxReach, reachDist[p]);

                /* 核心点继续扩展 */
                if (coreDist[p] < 1e18) {
                    for (int j = 0; j < n; ++j) {
                        if (processed[j]) continue;
                        double newRd = qMax(coreDist[p], dist[p][j]);
                        if (newRd < reachDist[j]) {
                            reachDist[j] = newRd;
                            seeds.append({newRd, j});
                        }
                    }
                }
            }
        }
    }

    /* 缓存排序结果供extractClusters使用 */
    m_orderingCache = ordering;

    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.totalClustered++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalClustered;

    emit orderingCompleted(ordering.size(), maxReach);
    return ordering;
}

/**
 * @brief 从可达距离排序中提取簇标签（陡度方法）
 *
 * 扫描可达距离图中的陡峭下降和上升区间，
 * 匹配下降-上升对形成簇。未归入任何簇的点标记为噪声(-1)。
 *
 * @param steepThreshold 陡度判定阈值
 * @return 每个样本的簇标签，噪声为-1
 */
QVector<int> OPTICS8::extractClusters(double steepThreshold)
{
    const int n = m_orderingCache.size();
    if (n == 0) return {};

    QVector<int> labels(n, -1);

    /* 识别陡峭下降区间和陡峭上升区间 */
    struct SteepRegion { int start; int end; bool isDown; };
    QVector<SteepRegion> regions;

    int i = 0;
    while (i < n - 1) {
        bool isDown = m_orderingCache[i + 1].second < m_orderingCache[i].second - steepThreshold;
        bool isUp = m_orderingCache[i + 1].second > m_orderingCache[i].second + steepThreshold;

        if (!isDown && !isUp) { ++i; continue; }

        int j = i;
        if (isDown) {
            while (j + 1 < n && m_orderingCache[j + 1].second < m_orderingCache[j].second - steepThreshold)
                ++j;
            if (j > i) regions.append({i, j, true});
        } else {
            while (j + 1 < n && m_orderingCache[j + 1].second > m_orderingCache[j].second + steepThreshold)
                ++j;
            if (j > i) regions.append({i, j, false});
        }
        i = qMax(i + 1, j + 1);
    }

    /* 匹配陡峭下降/上升对形成簇 */
    int clusterId = 0;
    for (int r = 0; r < regions.size(); ++r) {
        if (!regions[r].isDown) continue;
        for (int s = r + 1; s < regions.size(); ++s) {
            if (regions[s].isDown) continue;
            /* 匹配下降-上升对 */
            int cStart = regions[r].start;
            int cEnd = qMin(regions[s].end, n - 1);
            for (int p = cStart; p <= cEnd; ++p) {
                if (labels[p] < 0) labels[p] = clusterId;
            }
            clusterId++;
            break;
        }
    }

    return labels;
}
