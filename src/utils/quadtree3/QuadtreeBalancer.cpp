/**
 * @file QuadtreeBalancer.cpp
 * @brief 平衡四叉树实现
 */

#include "utils/quadtree3/QuadtreeBalancer.h"

#include <QElapsedTimer>
#include <algorithm>

/* ---- Node 实现 ---- */

QuadtreeOptBalancer::Node::Node()
    : x(0.0), y(0.0), value(0.0), occupied(false)
{
    for (int i = 0; i < 4; ++i) children[i] = nullptr;
    bounds[0] = bounds[1] = 0.0;
    bounds[2] = bounds[3] = 0.0;
}

QuadtreeOptBalancer::Node::~Node()
{
    for (int i = 0; i < 4; ++i) {
        delete children[i];
    }
}

/* ---- QuadtreeOptBalancer 实现 ---- */

QuadtreeOptBalancer::QuadtreeOptBalancer(QObject* parent)
    : QObject(parent)
    , m_timeSum(0.0)
    , m_size(16) /* 每个节点最多16个点触发分裂 */
{
    m_root = new Node();
    m_root->bounds[0] = -10000.0; /* xmin */
    m_root->bounds[1] = -10000.0; /* ymin */
    m_root->bounds[2] = 10000.0;  /* xmax */
    m_root->bounds[3] = 10000.0;  /* ymax */
}

QuadtreeOptBalancer::~QuadtreeOptBalancer()
{
    delete m_root;
}

bool QuadtreeOptBalancer::contains(Node* node, double x, double y) const
{
    return x >= node->bounds[0] && x < node->bounds[2] &&
           y >= node->bounds[1] && y < node->bounds[3];
}

bool QuadtreeOptBalancer::intersects(Node* node, double x1, double y1,
                                   double x2, double y2) const
{
    return !(x2 <= node->bounds[0] || x1 >= node->bounds[2] ||
             y2 <= node->bounds[1] || y1 >= node->bounds[3]);
}

int QuadtreeOptBalancer::getChildIndex(Node* node, double x, double y) const
{
    double midX = (node->bounds[0] + node->bounds[2]) / 2.0;
    double midY = (node->bounds[1] + node->bounds[3]) / 2.0;
    int idx = 0;
    if (x >= midX) idx |= 1;
    if (y >= midY) idx |= 2;
    return idx;
}

void QuadtreeOptBalancer::insertImpl(Node* node, double x, double y,
                                    double value)
{
    if (!contains(node, x, y)) return;

    /* 叶节点 */
    bool isLeaf = (node->children[0] == nullptr);

    if (isLeaf && !node->occupied) {
        node->x = x;
        node->y = y;
        node->value = value;
        node->occupied = true;
        return;
    }

    /* 需要分裂 */
    if (isLeaf) {
        double midX = (node->bounds[0] + node->bounds[2]) / 2.0;
        double midY = (node->bounds[1] + node->bounds[3]) / 2.0;

        double childBounds[4][4] = {
            {node->bounds[0], node->bounds[1], midX, midY},         /* NW */
            {midX, node->bounds[1], node->bounds[2], midY},         /* NE */
            {node->bounds[0], midY, midX, node->bounds[3]},         /* SW */
            {midX, midY, node->bounds[2], node->bounds[3]}          /* SE */
        };

        for (int i = 0; i < 4; ++i) {
            node->children[i] = new Node();
            for (int j = 0; j < 4; ++j) {
                node->children[i]->bounds[j] = childBounds[i][j];
            }
        }

        /* 重新插入原有点 */
        double ox = node->x, oy = node->y, ov = node->value;
        node->occupied = false;
        int oi = getChildIndex(node, ox, oy);
        insertImpl(node->children[oi], ox, oy, ov);
    }

    /* 插入新点 */
    int idx = getChildIndex(node, x, y);
    insertImpl(node->children[idx], x, y, value);
}

void QuadtreeOptBalancer::insert(double x, double y, double value)
{
    QElapsedTimer timer;
    timer.start();

    insertImpl(m_root, x, y, value);

    m_stats.totalInserts++;
    m_timeSum += timer.elapsed();
    double total = m_stats.totalInserts + m_stats.totalQueries +
                   m_stats.totalRebalances;
    m_stats.avgProcessingTimeMs = (total > 0) ? m_timeSum / total : 0.0;
}

void QuadtreeOptBalancer::rangeQueryImpl(
    Node* node, double x1, double y1, double x2, double y2,
    QVector<QPair<QPointF, double>>& result) const
{
    if (!intersects(node, x1, y1, x2, y2)) return;

    if (node->occupied) {
        if (node->x >= x1 && node->x <= x2 &&
            node->y >= y1 && node->y <= y2) {
            result.append({QPointF(node->x, node->y), node->value});
        }
    }

    bool isLeaf = (node->children[0] == nullptr);
    if (!isLeaf) {
        for (int i = 0; i < 4; ++i) {
            rangeQueryImpl(node->children[i], x1, y1, x2, y2, result);
        }
    }
}

QVector<QPair<QPointF, double>> QuadtreeOptBalancer::rangeQuery(
    double x1, double y1, double x2, double y2)
{
    QElapsedTimer timer;
    timer.start();

    QVector<QPair<QPointF, double>> result;
    rangeQueryImpl(m_root, x1, y1, x2, y2, result);

    m_stats.totalQueries++;
    m_timeSum += timer.elapsed();
    double total = m_stats.totalInserts + m_stats.totalQueries +
                   m_stats.totalRebalances;
    m_stats.avgProcessingTimeMs = (total > 0) ? m_timeSum / total : 0.0;

    return result;
}

void QuadtreeOptBalancer::collectPoints(
    Node* node, QVector<QPair<QPointF, double>>& points) const
{
    if (node->occupied) {
        points.append({QPointF(node->x, node->y), node->value});
    }
    bool isLeaf = (node->children[0] == nullptr);
    if (!isLeaf) {
        for (int i = 0; i < 4; ++i) {
            collectPoints(node->children[i], points);
        }
    }
}

void QuadtreeOptBalancer::clearNode(Node* node)
{
    for (int i = 0; i < 4; ++i) {
        delete node->children[i];
        node->children[i] = nullptr;
    }
    node->occupied = false;
}

void QuadtreeOptBalancer::rebalance()
{
    QElapsedTimer timer;
    timer.start();

    /* 收集所有点 */
    QVector<QPair<QPointF, double>> points;
    collectPoints(m_root, points);

    /* 按Hilbert曲线排序以保证空间局部性 */
    std::sort(points.begin(), points.end(),
              [](const auto& a, const auto& b) {
                  /* 简化: 用Z-order排序 */
                  auto zOrder = [](double x, double y) -> quint64 {
                      /* 将坐标映射到整数并交织位 */
                      quint32 ix = static_cast<quint32>(
                          (x + 10000.0) * 1000);
                      quint32 iy = static_cast<quint32>(
                          (y + 10000.0) * 1000);
                      quint64 z = 0;
                      for (int i = 0; i < 32; ++i) {
                          z |= static_cast<quint64>(
                              (ix & (1u << i))) << i;
                          z |= static_cast<quint64>(
                              (iy & (1u << i))) << (i + 1);
                      }
                      return z;
                  };
                  return zOrder(a.first.x(), a.first.y()) <
                         zOrder(b.first.x(), b.first.y());
              });

    /* 清空旧树 */
    clearNode(m_root);

    /* 按排序顺序重新插入(构建更平衡的树) */
    for (const auto& pt : points) {
        insertImpl(m_root, pt.first.x(), pt.first.y(), pt.second);
    }

    int nodes = nodeCount();

    m_stats.totalRebalances++;
    m_timeSum += timer.elapsed();
    double total = m_stats.totalInserts + m_stats.totalQueries +
                   m_stats.totalRebalances;
    m_stats.avgProcessingTimeMs = (total > 0) ? m_timeSum / total : 0.0;

    emit rebalanced(nodes);
}

int QuadtreeOptBalancer::countNodes(Node* node) const
{
    int count = node->occupied ? 1 : 0;
    bool isLeaf = (node->children[0] == nullptr);
    if (!isLeaf) {
        count += 4; /* 内部节点 */
        for (int i = 0; i < 4; ++i) {
            count += countNodes(node->children[i]);
        }
    }
    return count;
}

int QuadtreeOptBalancer::nodeCount() const
{
    return countNodes(m_root);
}

void QuadtreeOptBalancer::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
