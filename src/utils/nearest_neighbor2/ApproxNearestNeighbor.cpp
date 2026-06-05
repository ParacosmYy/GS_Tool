/**
 * @file ApproxNearestNeighbor.cpp
 * @brief LSH近似最近邻搜索实现
 */

#include "utils/nearest_neighbor2/ApproxNearestNeighbor.h"

#include <QElapsedTimer>
#include <QMap>
#include <algorithm>
#include <cmath>
#include <random>
#include <vector>

/** @brief 构造函数 @param numTables 哈希表数量 @param numHashes 每表哈希函数数 @param parent 父对象 */
ApproxNearestNeighbor::ApproxNearestNeighbor(int numTables, int numHashes,
                                               QObject* parent)
    : QObject(parent)
    , m_numTables(numTables)
    , m_numHashes(numHashes)
    , m_dimensions(0)
    , m_timeSum(0.0)
{
    m_tables.resize(m_numTables);
}

/** @brief 析构函数 */
ApproxNearestNeighbor::~ApproxNearestNeighbor() = default;

/**
 * @brief 从点集构建LSH索引
 *
 * 保存点集，为每个哈希表生成随机超平面，
 * 然后将所有点分配到对应的哈希桶中。
 */
void ApproxNearestNeighbor::build(const QVector<QVector<double>>& points)
{
    QElapsedTimer timer;
    timer.start();

    m_points = points;
    m_dimensions = 0;

    /* 清空哈希表 */
    for (int t = 0; t < m_numTables; ++t) {
        m_tables[t].buckets.clear();
        m_tables[t].hyperplanes.clear();
    }

    if (points.isEmpty()) {
        double elapsed = static_cast<double>(timer.elapsed());
        m_timeSum += elapsed;
        ++m_stats.totalBuilds;
        double total = static_cast<double>(m_stats.totalBuilds + m_stats.totalQueries);
        m_stats.avgProcessingTimeMs = (total > 0) ? m_timeSum / total : 0.0;
        return;
    }

    m_dimensions = points[0].size();
    generateHyperplanes();

    /* 将所有点分配到各哈希表的桶中 */
    for (int i = 0; i < points.size(); ++i) {
        if (points[i].size() != m_dimensions) continue;

        for (int t = 0; t < m_numTables; ++t) {
            QString key = hashPoint(points[i], t);
            m_tables[t].buckets[key].append(i);
        }
    }

    /* 更新统计 */
    double elapsed = static_cast<double>(timer.elapsed());
    m_timeSum += elapsed;
    ++m_stats.totalBuilds;
    double total = static_cast<double>(m_stats.totalBuilds + m_stats.totalQueries);
    m_stats.avgProcessingTimeMs = (total > 0) ? m_timeSum / total : 0.0;
}

/**
 * @brief 查询k个近似最近邻
 *
 * 对每个哈希表计算查询点的哈希值，收集候选点，
 * 然后按欧氏距离排序返回前k个。
 */
QVector<int> ApproxNearestNeighbor::query(const QVector<double>& point, int k)
{
    QElapsedTimer timer;
    timer.start();

    QVector<int> result;

    if (m_points.isEmpty() || point.size() != m_dimensions) {
        double elapsed = static_cast<double>(timer.elapsed());
        m_timeSum += elapsed;
        ++m_stats.totalQueries;
        double total = static_cast<double>(m_stats.totalBuilds + m_stats.totalQueries);
        m_stats.avgProcessingTimeMs = (total > 0) ? m_timeSum / total : 0.0;
        emit queryCompleted(0);
        return result;
    }

    /* 收集候选点(去重) */
    QMap<int, bool> candidateSet;
    for (int t = 0; t < m_numTables; ++t) {
        QString key = hashPoint(point, t);
        auto it = m_tables[t].buckets.find(key);
        if (it != m_tables[t].buckets.end()) {
            for (int idx : it.value()) {
                candidateSet[idx] = true;
            }
        }
    }

    /* 计算候选点与查询点的距离 */
    QVector<QPair<double, int>> distIdx;
    distIdx.reserve(candidateSet.size());
    for (auto it = candidateSet.begin(); it != candidateSet.end(); ++it) {
        double d = euclideanDist(point, m_points[it.key()]);
        distIdx.append({d, it.key()});
    }

    /* 按距离排序 */
    std::sort(distIdx.begin(), distIdx.end(),
              [](const QPair<double, int>& a, const QPair<double, int>& b) {
                  return a.first < b.first;
              });

    /* 取前k个 */
    int count = std::min(k, static_cast<int>(distIdx.size()));
    result.reserve(count);
    for (int i = 0; i < count; ++i) {
        result.append(distIdx[i].second);
    }

    /* 更新统计 */
    double elapsed = static_cast<double>(timer.elapsed());
    m_timeSum += elapsed;
    ++m_stats.totalQueries;
    double total = static_cast<double>(m_stats.totalBuilds + m_stats.totalQueries);
    m_stats.avgProcessingTimeMs = (total > 0) ? m_timeSum / total : 0.0;

    emit queryCompleted(result.size());
    return result;
}

/** @brief 重置统计信息 */
void ApproxNearestNeighbor::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

/**
 * @brief 计算点在指定哈希表中的哈希值
 *
 * 对每个哈希函数，计算点与超平面的点积符号(+1或-1)，
 * 组合为二进制哈希字符串。
 */
QString ApproxNearestNeighbor::hashPoint(const QVector<double>& point, int tableIdx)
{
    QString key;
    key.reserve(m_numHashes);

    const HashTable& table = m_tables[tableIdx];
    for (int h = 0; h < m_numHashes; ++h) {
        double dot = 0.0;
        const QVector<double>& plane = table.hyperplanes[h];
        int n = std::min(point.size(), plane.size());
        for (int d = 0; d < n; ++d) {
            dot += point[d] * plane[d];
        }
        key += (dot >= 0.0) ? '1' : '0';
    }
    return key;
}

/**
 * @brief 计算两个向量的欧氏距离
 */
double ApproxNearestNeighbor::euclideanDist(
    const QVector<double>& a, const QVector<double>& b) const
{
    double sum = 0.0;
    int n = std::min(a.size(), b.size());
    for (int i = 0; i < n; ++i) {
        double diff = a[i] - b[i];
        sum += diff * diff;
    }
    return std::sqrt(sum);
}

/**
 * @brief 为每个哈希表生成随机超平面法向量
 *
 * 使用标准正态分布生成随机法向量。
 * 每个哈希表有独立的超平面集合。
 */
void ApproxNearestNeighbor::generateHyperplanes()
{
    std::mt19937 gen(42); /* 固定种子保证可复现 */
    std::normal_distribution<double> dist(0.0, 1.0);

    for (int t = 0; t < m_numTables; ++t) {
        m_tables[t].hyperplanes.resize(m_numHashes);
        for (int h = 0; h < m_numHashes; ++h) {
            m_tables[t].hyperplanes[h].resize(m_dimensions);
            for (int d = 0; d < m_dimensions; ++d) {
                m_tables[t].hyperplanes[h][d] = dist(gen);
            }
        }
    }
}
