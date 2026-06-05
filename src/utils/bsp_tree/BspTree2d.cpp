/**
 * @file BspTree2d.cpp
 * @brief 二维BSP树实现 — 空间划分与范围查询
 */

#include "utils/bsp_tree/BspTree2d.h"

#include <QElapsedTimer>
#include <algorithm>

// ============================================================================
// 构造/析构
// ============================================================================

BspTree2d::BspTree2d(int maxLeafSize, QObject* parent)
    : QObject(parent)
    , m_root(new Node())
    , m_maxLeafSize(maxLeafSize)
    , m_size(0)
    , m_timeSum(0.0)
{
}

BspTree2d::~BspTree2d()
{
    deleteNode(m_root);
}

void BspTree2d::deleteNode(Node* node)
{
    if (!node) return;
    deleteNode(node->left);
    deleteNode(node->right);
    delete node;
}

// ============================================================================
// 公开方法
// ============================================================================

bool BspTree2d::insert(double x, double y, int data)
{
    QElapsedTimer timer;
    timer.start();

    insertImpl(m_root, x, y, data, 0);
    ++m_size;

    ++m_stats.totalInserts;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs =
        (m_stats.totalInserts + m_stats.totalQueries > 0)
        ? m_timeSum / (m_stats.totalInserts + m_stats.totalQueries) : 0.0;

    return true;
}

QVector<QPair<QPair<double,double>,int>> BspTree2d::rangeQuery(
    double xMin, double yMin, double xMax, double yMax)
{
    QElapsedTimer timer;
    timer.start();

    QVector<QPair<QPair<double,double>,int>> results;
    rangeQueryImpl(m_root, xMin, yMin, xMax, yMax, results);

    ++m_stats.totalQueries;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs =
        (m_stats.totalInserts + m_stats.totalQueries > 0)
        ? m_timeSum / (m_stats.totalInserts + m_stats.totalQueries) : 0.0;

    emit queryCompleted(results.size());
    return results;
}

void BspTree2d::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

void BspTree2d::clear()
{
    deleteNode(m_root);
    m_root = new Node();
    m_size = 0;
}

// ============================================================================
// 内部实现
// ============================================================================

void BspTree2d::insertImpl(Node* node, double x, double y, int data, int depth)
{
    /* 叶节点: 直接添加点 */
    if (node->isLeaf) {
        node->points.append({{x, y}, data});

        /* 超过容量则分裂 */
        if (static_cast<int>(node->points.size()) > m_maxLeafSize) {
            splitNode(node, depth);
        }
        return;
    }

    /* 内部节点: 沿分割轴递归 */
    double coord = (node->splitAxis == 0) ? x : y;
    if (coord <= node->splitPos) {
        insertImpl(node->left, x, y, data, depth + 1);
    } else {
        insertImpl(node->right, x, y, data, depth + 1);
    }
}

void BspTree2d::splitNode(Node* node, int depth)
{
    int axis = depth % 2; // 0=X, 1=Y 交替分割
    auto& pts = node->points;
    int n = pts.size();
    if (n < 2) return;

    /* 按当前轴排序，取中值作为分割点 */
    std::sort(pts.begin(), pts.end(),
        [axis](const QPair<QPair<double,double>,int>& a,
               const QPair<QPair<double,double>,int>& b) {
            return (axis == 0) ? (a.first.first < b.first.first)
                               : (a.first.second < b.first.second);
        });

    int mid = n / 2;
    node->splitAxis = axis;
    node->splitPos = (axis == 0) ? pts[mid].first.first
                                  : pts[mid].first.second;
    node->isLeaf = false;

    /* 创建子节点 */
    node->left = new Node();
    node->right = new Node();

    /* 分配点到子节点 */
    for (int i = 0; i < n; ++i) {
        double coord = (axis == 0) ? pts[i].first.first
                                    : pts[i].first.second;
        if (coord <= node->splitPos) {
            node->left->points.append(pts[i]);
        } else {
            node->right->points.append(pts[i]);
        }
    }

    /* 清空当前节点的点 */
    node->points.clear();
}

void BspTree2d::rangeQueryImpl(
    Node* node, double xMin, double yMin, double xMax, double yMax,
    QVector<QPair<QPair<double,double>,int>>& results)
{
    if (!node) return;

    /* 叶节点: 逐点检查 */
    if (node->isLeaf) {
        for (const auto& pt : node->points) {
            double px = pt.first.first;
            double py = pt.first.second;
            if (px >= xMin && px <= xMax && py >= yMin && py <= yMax) {
                results.append(pt);
            }
        }
        return;
    }

    /* 内部节点: 根据分割轴决定搜索方向 */
    double lo = (node->splitAxis == 0) ? xMin : yMin;
    double hi = (node->splitAxis == 0) ? xMax : yMax;

    if (lo <= node->splitPos) {
        rangeQueryImpl(node->left, xMin, yMin, xMax, yMax, results);
    }
    if (hi > node->splitPos) {
        rangeQueryImpl(node->right, xMin, yMin, xMax, yMax, results);
    }
}
