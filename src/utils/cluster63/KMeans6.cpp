/**
 * @file KMeans6.cpp
 * @brief K-Means聚类算法实现（第6版）
 *
 * 实现经典K-Means聚类算法，支持随机初始化和K-Means++初始化策略。
 * 使用欧氏距离度量，迭代优化直到收敛或达到最大迭代次数。
 * 输出每个数据点的簇标签和质心坐标。
 *
 * @author EmbedDebug Team
 * @date 2026-06-05
 */

#include "utils/cluster63/KMeans6.h"

#include <QElapsedTimer>
#include <QtMath>
#include <random>

/**
 * @brief 构造函数，初始化K-Means聚类器
 * @param parent 父QObject对象指针
 */
KMeans6::KMeans6(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置簇的数量
 * @param k 簇数量，至少为1
 */
void KMeans6::setNumClusters(int k)
{
    m_k = qMax(1, k);
}

/**
 * @brief 设置最大迭代次数
 * @param iter 最大迭代次数
 */
void KMeans6::setMaxIterations(int iter)
{
    m_maxIter = qMax(1, iter);
}

/**
 * @brief 设置初始化方法
 * @param method 初始化策略："random"或"kmeans++"
 */
void KMeans6::setInitMethod(const QString& method)
{
    if (method == "random" || method == "kmeans++") {
        m_init = method;
    }
}

/**
 * @brief 设置随机种子
 * @param seed 随机数种子值
 */
void KMeans6::setSeed(unsigned int seed)
{
    m_seed = seed;
}

/**
 * @brief 计算两点之间的欧氏距离平方
 * @param a 第一个点
 * @param b 第二个点
 * @return 欧氏距离的平方
 */
double KMeans6::distance(const QVector<double>& a, const QVector<double>& b) const
{
    double sum = 0.0;
    int dim = qMin(a.size(), b.size());
    for (int i = 0; i < dim; ++i) {
        double d = a[i] - b[i];
        sum += d * d;
    }
    return sum;
}

/**
 * @brief 随机初始化质心
 * @param pts 数据点集合
 */
void KMeans6::initRandom(const QVector<QVector<double>>& pts)
{
    std::mt19937 gen(m_seed);
    std::uniform_int_distribution<int> dist(0, pts.size() - 1);

    m_centroids.clear();
    QSet<int> chosen;
    int attempts = 0;
    while (m_centroids.size() < m_k && attempts < m_k * 10) {
        int idx = dist(gen);
        if (!chosen.contains(idx)) {
            m_centroids.append(pts[idx]);
            chosen.insert(idx);
        }
        attempts++;
    }
    /* 若点数不足k个，用重复点填充 */
    while (m_centroids.size() < m_k) {
        m_centroids.append(pts[0]);
    }
}

/**
 * @brief K-Means++初始化质心
 *
 * 按距离概率选择下一个质心，使初始质心分布更均匀，
 * 从而加速收敛并改善结果质量。
 *
 * @param pts 数据点集合
 */
void KMeans6::initKMeansPP(const QVector<QVector<double>>& pts)
{
    std::mt19937 gen(m_seed);

    m_centroids.clear();

    /* 随机选择第一个质心 */
    std::uniform_int_distribution<int> dist(0, pts.size() - 1);
    m_centroids.append(pts[dist(gen)]);

    /* 依次选择后续质心 */
    QVector<double> minDist(pts.size(), 0.0);
    while (m_centroids.size() < m_k) {
        /* 计算每个点到最近质心的距离 */
        double totalDist = 0.0;
        for (int i = 0; i < pts.size(); ++i) {
            double md = std::numeric_limits<double>::max();
            for (const auto& c : m_centroids) {
                md = qMin(md, distance(pts[i], c));
            }
            minDist[i] = md;
            totalDist += md;
        }

        if (totalDist < 1e-15) {
            /* 所有点重合 */
            m_centroids.append(pts[0]);
            continue;
        }

        /* 按距离的概率选择下一个质心 */
        std::uniform_real_distribution<double> rDist(0.0, totalDist);
        double r = rDist(gen);
        double cumSum = 0.0;
        int chosen = 0;
        for (int i = 0; i < pts.size(); ++i) {
            cumSum += minDist[i];
            if (cumSum >= r) {
                chosen = i;
                break;
            }
        }
        m_centroids.append(pts[chosen]);
    }
}

/**
 * @brief 执行K-Means聚类
 *
 * 算法流程：
 * 1. 使用指定方法初始化质心
 * 2. 分配每个点到最近质心
 * 3. 重新计算每个簇的质心
 * 4. 重复2-3直到收敛或达到最大迭代
 *
 * @param points 输入数据点集合
 * @return 每个点的簇标签（0~k-1）
 */
QVector<int> KMeans6::cluster(const QVector<QVector<double>>& points)
{
    QElapsedTimer timer;
    timer.start();

    QVector<int> labels;

    if (points.isEmpty() || m_k <= 0) {
        emit clusteringCompleted(0, 0.0);
        return labels;
    }

    int n = points.size();
    labels.resize(n, 0);

    /* 限制k不超过点数 */
    int actualK = qMin(m_k, n);

    /* 初始化质心 */
    if (m_init == "kmeans++") {
        initKMeansPP(points);
    } else {
        initRandom(points);
    }

    /* 迭代优化 */
    m_inertia = 0.0;
    for (int iter = 0; iter < m_maxIter; ++iter) {
        /* 分配步骤：将每个点分配到最近质心 */
        double newInertia = 0.0;
        bool changed = false;

        for (int i = 0; i < n; ++i) {
            double minD = std::numeric_limits<double>::max();
            int bestC = 0;
            for (int c = 0; c < actualK; ++c) {
                double d = distance(points[i], m_centroids[c]);
                if (d < minD) {
                    minD = d;
                    bestC = c;
                }
            }
            if (labels[i] != bestC) {
                changed = true;
                labels[i] = bestC;
            }
            newInertia += minD;
        }

        m_inertia = newInertia;

        /* 若无变化则收敛 */
        if (!changed) break;

        /* 更新步骤：重新计算质心 */
        int dim = points[0].size();
        QVector<QVector<double>> newCentroids(actualK, QVector<double>(dim, 0.0));
        QVector<int> counts(actualK, 0);

        for (int i = 0; i < n; ++i) {
            int c = labels[i];
            counts[c]++;
            for (int d = 0; d < dim; ++d) {
                newCentroids[c][d] += points[i][d];
            }
        }

        for (int c = 0; c < actualK; ++c) {
            if (counts[c] > 0) {
                for (int d = 0; d < dim; ++d) {
                    newCentroids[c][d] /= counts[c];
                }
                m_centroids[c] = newCentroids[c];
            }
        }
    }

    /* 更新统计 */
    m_stats.totalClusterings++;
    m_stats.totalPoints += n;
    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalClusterings;

    emit clusteringCompleted(actualK, m_inertia);
    return labels;
}

/**
 * @brief 获取当前统计信息
 * @return 聚类统计结构
 */
KMeans6::Stats KMeans6::stats() const
{
    return m_stats;
}

/**
 * @brief 重置所有统计数据
 */
void KMeans6::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
