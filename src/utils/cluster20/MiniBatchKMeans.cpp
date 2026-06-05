/**
 * @file MiniBatchKMeans.cpp
 * @brief 小批量K均值聚类实现 — SGD优化+批量分配+收敛追踪
 */

#include "utils/cluster20/MiniBatchKMeans.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <random>

/** @brief 构造函数 @param parent 父对象 */
MiniBatchKMeans::MiniBatchKMeans(QObject* parent)
    : QObject(parent)
    , m_k(8)
    , m_batchSize(100)
    , m_maxIterations(100)
    , m_initMethod(InitMethod::KMeansPlusPlus)
{
}

void MiniBatchKMeans::setClusterCount(int k) { m_k = qMax(2, k); }
void MiniBatchKMeans::setBatchSize(int size) { m_batchSize = qMax(1, size); }
void MiniBatchKMeans::setMaxIterations(int iterations) { m_maxIterations = qMax(1, iterations); }
void MiniBatchKMeans::setInitMethod(InitMethod method) { m_initMethod = method; }

/**
 * @brief 完整拟合数据集
 * @param data 输入数据集(每行为一个样本)
 */
void MiniBatchKMeans::fit(const QVector<QVector<double>>& data)
{
    if (data.size() < m_k) return;

    QElapsedTimer timer;
    timer.start();

    initializeCentroids(data);
    m_counts.assign(m_k, 0);

    double prevInertia = 1e300;
    for (int iter = 0; iter < m_maxIterations; ++iter) {
        /* 从数据中随机抽取一个小批量 */
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<int> dist(0, data.size() - 1);

        QVector<QVector<double>> batch;
        batch.reserve(m_batchSize);
        for (int i = 0; i < m_batchSize && i < data.size(); ++i) {
            batch.append(data[dist(gen)]);
        }

        updateCentroids(batch);

        double inertia = computeInertia(data);
        ++m_stats.iterationsRun;

        emit iterationCompleted(iter, inertia);

        /* 收敛检测: 惯性变化小于阈值 */
        if (qAbs(prevInertia - inertia) < 1e-6 * prevInertia && prevInertia > 0) {
            break;
        }
        prevInertia = inertia;
    }

    m_stats.totalInertia = prevInertia;

    double elapsed = static_cast<double>(timer.elapsed());
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(qMax(1ULL, m_stats.totalBatches));

    emit fittingFinished(m_stats.iterationsRun, prevInertia);
}

/**
 * @brief 在线增量学习一个批次
 * @param batch 新到达的数据批次
 */
void MiniBatchKMeans::partialFit(const QVector<QVector<double>>& batch)
{
    QElapsedTimer timer;
    timer.start();

    if (m_centroids.isEmpty() && batch.size() >= m_k) {
        initializeCentroids(batch);
        m_counts.assign(m_k, 0);
    }

    if (!m_centroids.isEmpty()) {
        updateCentroids(batch);
    }

    double elapsed = static_cast<double>(timer.elapsed());
    ++m_stats.totalBatches;
    m_stats.totalPointsAssigned += static_cast<quint64>(batch.size());
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(m_stats.totalBatches);
}

/**
 * @brief 预测数据点的聚类归属
 * @param data 输入数据集
 * @return 聚类结果列表
 */
QList<MiniBatchKMeans::ClusterResult> MiniBatchKMeans::predict(
    const QVector<QVector<double>>& data) const
{
    QList<ClusterResult> results;
    for (const auto& point : data) {
        ClusterResult r;
        r.clusterId = findNearestCluster(point);
        if (r.clusterId >= 0) {
            r.distance = euclideanDistance(point, m_centroids[r.clusterId]);
        }
        results.append(r);
    }
    return results;
}

/** @brief 获取当前聚类中心 @return 聚类中心向量 */
QVector<QVector<double>> MiniBatchKMeans::centroids() const { return m_centroids; }

/** @brief 重置统计信息 */
void MiniBatchKMeans::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

/**
 * @brief 初始化聚类中心
 * @param data 输入数据
 */
void MiniBatchKMeans::initializeCentroids(const QVector<QVector<double>>& data)
{
    int dims = data[0].size();
    m_centroids.clear();
    m_centroids.reserve(m_k);

    std::random_device rd;
    std::mt19937 gen(rd());

    if (m_initMethod == InitMethod::Random) {
        /* 随机选取k个样本作为初始中心 */
        QVector<int> indices(data.size());
        std::iota(indices.begin(), indices.end(), 0);
        std::shuffle(indices.begin(), indices.end(), gen);

        for (int i = 0; i < m_k && i < data.size(); ++i) {
            m_centroids.append(data[indices[i]]);
        }
    } else {
        /* K-means++: 距离加权选取 */
        std::uniform_int_distribution<int> dist(0, data.size() - 1);
        m_centroids.append(data[dist(gen)]);

        QVector<double> minDists(data.size(), 1e300);
        for (int c = 1; c < m_k; ++c) {
            /* 计算每个点到最近中心的距离 */
            double distSum = 0.0;
            for (int i = 0; i < data.size(); ++i) {
                double d = euclideanDistance(data[i], m_centroids.last());
                if (d < minDists[i]) minDists[i] = d;
                distSum += minDists[i];
            }

            /* 按距离平方概率选取下一个中心 */
            std::uniform_real_distribution<double> rDist(0.0, distSum);
            double threshold = rDist(gen);
            double cumulative = 0.0;
            int chosen = 0;
            for (int i = 0; i < data.size(); ++i) {
                cumulative += minDists[i];
                if (cumulative >= threshold) { chosen = i; break; }
            }
            m_centroids.append(data[chosen]);
        }
    }
}

/**
 * @brief 用小批量更新聚类中心(SGD步骤)
 * @param batch 小批量数据
 */
void MiniBatchKMeans::updateCentroids(const QVector<QVector<double>>& batch)
{
    /* 为批量中每个点找到最近中心，累积数据 */
    QVector<QVector<double>> sums(m_k);
    QVector<int> batchCounts(m_k, 0);
    for (int i = 0; i < m_k; ++i) {
        sums[i].resize(m_centroids[i].size());
        std::fill(sums[i].begin(), sums[i].end(), 0.0);
    }

    for (const auto& point : batch) {
        int nearest = findNearestCluster(point);
        if (nearest < 0) continue;
        for (int d = 0; d < point.size(); ++d) {
            sums[nearest][d] += point[d];
        }
        ++batchCounts[nearest];
    }

    /* 流式更新中心: c = c + (sum/count - c) * (count / (count + batchCount)) */
    for (int i = 0; i < m_k; ++i) {
        if (batchCounts[i] == 0) continue;
        int newCount = m_counts[i] + batchCounts[i];
        double lr = static_cast<double>(batchCounts[i])
            / static_cast<double>(newCount);
        for (int d = 0; d < m_centroids[i].size(); ++d) {
            double batchMean = sums[i][d] / batchCounts[i];
            m_centroids[i][d] += lr * (batchMean - m_centroids[i][d]);
        }
        m_counts[i] = newCount;
    }
}

/**
 * @brief 计算总惯性(误差平方和)
 * @param data 数据集
 * @return 惯性值
 */
double MiniBatchKMeans::computeInertia(const QVector<QVector<double>>& data) const
{
    double inertia = 0.0;
    for (const auto& point : data) {
        int nearest = findNearestCluster(point);
        if (nearest >= 0) {
            double d = euclideanDistance(point, m_centroids[nearest]);
            inertia += d * d;
        }
    }
    return inertia;
}

/**
 * @brief 找到最近聚类
 * @param point 数据点
 * @return 聚类ID
 */
int MiniBatchKMeans::findNearestCluster(const QVector<double>& point) const
{
    if (m_centroids.isEmpty()) return -1;
    int nearest = 0;
    double minDist = euclideanDistance(point, m_centroids[0]);
    for (int i = 1; i < m_centroids.size(); ++i) {
        double d = euclideanDistance(point, m_centroids[i]);
        if (d < minDist) { minDist = d; nearest = i; }
    }
    return nearest;
}

/**
 * @brief 计算欧几里得距离
 * @param a 向量A
 * @param b 向量B
 * @return 距离值
 */
double MiniBatchKMeans::euclideanDistance(const QVector<double>& a,
                                         const QVector<double>& b) const
{
    double sum = 0.0;
    int dims = qMin(a.size(), b.size());
    for (int i = 0; i < dims; ++i) {
        double diff = a[i] - b[i];
        sum += diff * diff;
    }
    return qSqrt(sum);
}
