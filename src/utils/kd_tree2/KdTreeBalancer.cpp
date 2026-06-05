/**
 * @file KdTreeBalancer.cpp
 * @brief 平衡KD树实现
 */

#include "utils/kd_tree2/KdTreeBalancer.h"

#include <QElapsedTimer>
#include <algorithm>
#include <cmath>

/** @brief 构造函数 @param parent 父对象 */
KdTreeBalancer::KdTreeBalancer(QObject* parent)
    : QObject(parent)
    , m_root(nullptr)
    , m_size(0)
    , m_timeSum(0.0)
{
}

/** @brief 析构函数 */
KdTreeBalancer::~KdTreeBalancer()
{
    destroyTree(m_root);
}

/**
 * @brief 从点集构建平衡KD树
 *
 * 将输入点包装为内部格式(保存原始索引)，
 * 递归按中位数分割构建平衡树。
 */
void KdTreeBalancer::build(const QVector<QPair<double, double>>& points)
{
    QElapsedTimer timer;
    timer.start();

    destroyTree(m_root);
    m_root = nullptr;
    m_size = 0;

    if (points.isEmpty()) {
        double elapsed = static_cast<double>(timer.elapsed());
        m_timeSum += elapsed;
        ++m_stats.totalBuilds;
        double total = static_cast<double>(m_stats.totalBuilds + m_stats.totalQueries);
        m_stats.avgProcessingTimeMs = (total > 0) ? m_timeSum / total : 0.0;
        return;
    }

    /* 包装点集，保存原始索引 */
    QVector<Point2D> internalPts;
    internalPts.reserve(points.size());
    for (int i = 0; i < points.size(); ++i) {
        internalPts.append({points[i].first, points[i].second, i});
    }

    m_root = buildNode(internalPts, 0, internalPts.size() - 1, 0);
    m_size = points.size();

    /* 更新统计 */
    double elapsed = static_cast<double>(timer.elapsed());
    m_timeSum += elapsed;
    ++m_stats.totalBuilds;
    double total = static_cast<double>(m_stats.totalBuilds + m_stats.totalQueries);
    m_stats.avgProcessingTimeMs = (total > 0) ? m_timeSum / total : 0.0;
}

/**
 * @brief 最近邻查询
 *
 * 递归搜索KD树，维护当前最近点和距离。
 * 通过剪枝(超平面距离判断)减少搜索空间。
 */
QPair<int, double> KdTreeBalancer::nearestNeighbor(double x, double y)
{
    QElapsedTimer timer;
    timer.start();

    QPair<int, double> result = {-1, -1.0};

    if (!m_root) {
        double elapsed = static_cast<double>(timer.elapsed());
        m_timeSum += elapsed;
        ++m_stats.totalQueries;
        double total = static_cast<double>(m_stats.totalBuilds + m_stats.totalQueries);
        m_stats.avgProcessingTimeMs = (total > 0) ? m_timeSum / total : 0.0;
        emit queryCompleted(-1.0);
        return result;
    }

    int bestIdx = -1;
    double bestDist = 1e18;
    nearestNode(m_root, x, y, 0, bestIdx, bestDist);

    double dist = std::sqrt(bestDist);
    result = {bestIdx, dist};

    /* 更新统计 */
    double elapsed = static_cast<double>(timer.elapsed());
    m_timeSum += elapsed;
    ++m_stats.totalQueries;
    double total = static_cast<double>(m_stats.totalBuilds + m_stats.totalQueries);
    m_stats.avgProcessingTimeMs = (total > 0) ? m_timeSum / total : 0.0;

    emit queryCompleted(dist);
    return result;
}

/** @brief 重置统计信息 */
void KdTreeBalancer::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

/**
 * @brief 递归建树
 *
 * 使用std::nth_element进行O(n)中位数查找，
 * 保证树高度为O(log n)，实现平衡KD树。
 */
KdTreeBalancer::Node* KdTreeBalancer::buildNode(
    QVector<Point2D>& points, int start, int end, int depth)
{
    if (start > end) return nullptr;

    int axis = depth % 2; /* 0=x, 1=y */
    int mid = (start + end) / 2;

    /* 按当前轴的中位数分割 */
    std::nth_element(
        points.begin() + start, points.begin() + mid,
        points.begin() + end + 1,
        [axis](const Point2D& a, const Point2D& b) {
            return (axis == 0) ? (a.x < b.x) : (a.y < b.y);
        });

    Node* node = new Node;
    node->point = points[mid];
    node->left = buildNode(points, start, mid - 1, depth + 1);
    node->right = buildNode(points, mid + 1, end, depth + 1);
    return node;
}

/**
 * @brief 递归最近邻搜索
 *
 * 先搜索较近的子树，然后判断是否需要搜索较远的子树。
 * bestDist存储的是距离平方以避免重复sqrt计算。
 */
void KdTreeBalancer::nearestNode(Node* node, double x, double y, int depth,
                                  int& bestIdx, double& bestDist)
{
    if (!node) return;

    /* 计算到当前节点的距离平方 */
    double dx = x - node->point.x;
    double dy = y - node->point.y;
    double d = dx * dx + dy * dy;

    if (d < bestDist) {
        bestDist = d;
        bestIdx = node->point.origIndex;
    }

    int axis = depth % 2;
    double diff = (axis == 0) ? (x - node->point.x) : (y - node->point.y);

    Node* nearChild = (diff < 0) ? node->left : node->right;
    Node* farChild = (diff < 0) ? node->right : node->left;

    /* 先搜索较近的子树 */
    nearestNode(nearChild, x, y, depth + 1, bestIdx, bestDist);

    /* 判断是否需要搜索较远的子树 */
    if (diff * diff < bestDist) {
        nearestNode(farChild, x, y, depth + 1, bestIdx, bestDist);
    }
}

/** @brief 递归销毁树 */
void KdTreeBalancer::destroyTree(Node* node)
{
    if (!node) return;
    destroyTree(node->left);
    destroyTree(node->right);
    delete node;
}
