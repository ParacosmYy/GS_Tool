/**
 * @file KDTree.cpp
 * @brief KD树实现 — k维空间索引
 */

#include "utils/kdtree/KDTree.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/** @brief 构造函数 @param dimensions 维度 @param parent 父对象 */
KDTree::KDTree(int dimensions, QObject* parent)
    : QObject(parent)
    , m_dimensions(dimensions)
    , m_root(nullptr)
    , m_size(0)
    , m_timeSum(0.0)
{
}

/** @brief 析构函数 */
KDTree::~KDTree()
{
    destroyTree(m_root);
}

/** @brief 插入点 */
void KDTree::insert(const Point& point)
{
    QElapsedTimer timer;
    timer.start();

    m_root = insertNode(m_root, point, 0);
    ++m_size;

    double elapsed = static_cast<double>(timer.elapsed());
    m_timeSum += elapsed;
    ++m_stats.totalInserts;
    double total = static_cast<double>(m_stats.totalInserts + m_stats.totalQueries);
    m_stats.avgProcessingTimeMs = (total > 0) ? m_timeSum / total : 0.0;

    emit insertCompleted(m_size);
}

/** @brief 批量建树 */
void KDTree::build(const QVector<Point>& points)
{
    QElapsedTimer timer;
    timer.start();

    destroyTree(m_root);
    m_root = nullptr;

    QVector<Point> pts = points;
    m_size = 0;
    m_root = buildNode(pts, 0, pts.size() - 1, 0);
    m_size = points.size();

    double elapsed = static_cast<double>(timer.elapsed());
    m_timeSum += elapsed;
    m_stats.totalInserts += static_cast<quint64>(points.size());
    double total = static_cast<double>(m_stats.totalInserts + m_stats.totalQueries);
    m_stats.avgProcessingTimeMs = (total > 0) ? m_timeSum / total : 0.0;

    emit insertCompleted(m_size);
}

/** @brief 最近邻查询 */
KDTree::Point KDTree::nearestNeighbor(const Point& target) const
{
    if (!m_root) return Point();

    Node* best = nullptr;
    double bestDist = 1e18;
    nearestNode(m_root, target, 0, best, bestDist);
    return best ? best->point : Point();
}

/** @brief K近邻查询 */
QVector<KDTree::Point> KDTree::kNearestNeighbors(const Point& target,
                                                   int k) const
{
    /* 使用优先队列的简化实现 */
    QVector<QPair<double, Point>> candidates;

    /* 中序遍历收集所有点 */
    QVector<Node*> stack;
    Node* cur = m_root;
    while (cur || !stack.isEmpty()) {
        while (cur) {
            stack.append(cur);
            cur = cur->left;
        }
        cur = stack.takeLast();
        double d = distanceSquared(target, cur->point);
        candidates.append({d, cur->point});
        cur = cur->right;
    }

    /* 排序取前k个 */
    std::sort(candidates.begin(), candidates.end(),
              [](const auto& a, const auto& b) { return a.first < b.first; });

    QVector<Point> result;
    for (int i = 0; i < qMin(k, candidates.size()); ++i)
        result.append(candidates[i].second);
    return result;
}

/** @brief 范围查询 */
QVector<KDTree::Point> KDTree::rangeQuery(const Point& minCorner,
                                           const Point& maxCorner) const
{
    QVector<Point> result;
    rangeNode(m_root, minCorner, maxCorner, 0, result);
    return result;
}

/** @brief 重置统计 */
void KDTree::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

/** @brief 距离平方 */
double KDTree::distanceSquared(const Point& a, const Point& b) const
{
    double sum = 0.0;
    int n = qMin(a.size(), b.size());
    for (int i = 0; i < n; ++i) {
        double d = a[i] - b[i];
        sum += d * d;
    }
    return sum;
}

/** @brief 递归插入 */
KDTree::Node* KDTree::insertNode(Node* node, const Point& point, int depth)
{
    if (!node) return new Node{point, nullptr, nullptr};

    int axis = depth % m_dimensions;
    if (point[axis] < node->point[axis])
        node->left = insertNode(node->left, point, depth + 1);
    else
        node->right = insertNode(node->right, point, depth + 1);

    return node;
}

/** @brief 递归建树 */
KDTree::Node* KDTree::buildNode(QVector<Point>& points,
                                 int start, int end, int depth)
{
    if (start > end) return nullptr;

    int axis = depth % m_dimensions;
    int mid = (start + end) / 2;

    /* 按当前轴中位数分割 */
    std::nth_element(points.begin() + start, points.begin() + mid,
                     points.begin() + end + 1,
                     [axis](const Point& a, const Point& b) {
                         return a[axis] < b[axis];
                     });

    Node* node = new Node{points[mid], nullptr, nullptr};
    node->left = buildNode(points, start, mid - 1, depth + 1);
    node->right = buildNode(points, mid + 1, end, depth + 1);
    return node;
}

/** @brief 递归最近邻 */
void KDTree::nearestNode(Node* node, const Point& target, int depth,
                          Node*& best, double& bestDist) const
{
    if (!node) return;

    double d = distanceSquared(target, node->point);
    if (d < bestDist) {
        bestDist = d;
        best = node;
    }

    int axis = depth % m_dimensions;
    double diff = target[axis] - node->point[axis];

    Node* near = (diff < 0) ? node->left : node->right;
    Node* far = (diff < 0) ? node->right : node->left;

    nearestNode(near, target, depth + 1, best, bestDist);

    if (diff * diff < bestDist)
        nearestNode(far, target, depth + 1, best, bestDist);
}

/** @brief 递归范围查询 */
void KDTree::rangeNode(Node* node, const Point& minC, const Point& maxC,
                        int depth, QVector<Point>& result) const
{
    if (!node) return;

    int axis = depth % m_dimensions;
    bool inRange = true;
    for (int d = 0; d < m_dimensions; ++d) {
        if (node->point[d] < minC[d] || node->point[d] > maxC[d]) {
            inRange = false;
            break;
        }
    }
    if (inRange) result.append(node->point);

    if (node->point[axis] >= minC[axis])
        rangeNode(node->left, minC, maxC, depth + 1, result);
    if (node->point[axis] <= maxC[axis])
        rangeNode(node->right, minC, maxC, depth + 1, result);
}

/** @brief 递归销毁 */
void KDTree::destroyTree(Node* node)
{
    if (!node) return;
    destroyTree(node->left);
    destroyTree(node->right);
    delete node;
}
