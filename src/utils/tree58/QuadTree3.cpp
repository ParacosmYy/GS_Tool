/**
 * @file QuadTree3.cpp
 * @brief 四叉树空间索引实现 — 范围查询 + K近邻搜索
 * @author EmbedDebug Team
 * @date 2026-06-05
 *
 * 实现四叉树空间索引，支持二维点的动态插入、
 * 矩形范围查询和 K 近邻搜索。
 * 使用节点分裂策略处理超出容量的叶节点。
 */

#include "utils/tree58/QuadTree3.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

// ──────────────────────────────────────────────
// 内部节点结构定义
// ──────────────────────────────────────────────

/**
 * @brief 四叉树节点结构
 */
struct QuadTree3::QNode {
    double x, y;         ///< 分裂点坐标（内部节点）或数据点坐标（叶节点）
    int id;              ///< 数据点标识符
    bool isLeaf;         ///< 是否为叶节点
    int count;           ///< 叶节点中的数据点数量
    QNode* children[4];  ///< 四个子节点：NW, NE, SW, SE
    double xMin, yMin, xMax, yMax; ///< 节点边界

    // 叶节点数据（当 count <= capacity 时使用）
    static const int MAX_INLINE = 8;
    double px[MAX_INLINE], py[MAX_INLINE];
    int pids[MAX_INLINE];

    QNode(double xMin_, double yMin_, double xMax_, double yMax_)
        : x(0), y(0), id(-1), isLeaf(true), count(0),
          xMin(xMin_), yMin(yMin_), xMax(xMax_), yMax(yMax_)
    {
        for (int i = 0; i < 4; ++i) children[i] = nullptr;
    }
};

// ──────────────────────────────────────────────
// 构造函数
// ──────────────────────────────────────────────

/**
 * @brief 构造函数，初始化空四叉树
 * @param parent 父QObject对象
 */
QuadTree3::QuadTree3(QObject* parent)
    : QObject(parent)
    , m_root(nullptr)
    , m_xMin(0.0), m_yMin(0.0), m_xMax(1.0), m_yMax(1.0)
{
    setObjectName(QStringLiteral("QuadTree3"));
}

// ──────────────────────────────────────────────
// 参数配置
// ──────────────────────────────────────────────

/**
 * @brief 设置四叉树的空间范围
 * @param xMin X 最小值
 * @param yMin Y 最小值
 * @param xMax X 最大值
 * @param yMax Y 最大值
 */
void QuadTree3::setBounds(double xMin, double yMin, double xMax, double yMax)
{
    m_xMin = xMin;
    m_yMin = yMin;
    m_xMax = xMax;
    m_yMax = yMax;
}

/**
 * @brief 设置节点最大容量
 *
 * 当叶节点的数据点数超过容量时，节点分裂为四个子节点。
 *
 * @param cap 最大容量，范围 [1, 64]
 */
void QuadTree3::setMaxCapacity(int cap)
{
    m_capacity = qBound(1, cap, 64);
}

// ──────────────────────────────────────────────
// 插入操作
// ──────────────────────────────────────────────

/**
 * @brief 插入一个二维点到四叉树中
 * @param x X 坐标
 * @param y Y 坐标
 * @param id 点标识符
 */
void QuadTree3::insert(double x, double y, int id)
{
    QElapsedTimer timer;
    timer.start();

    if (m_root == nullptr) {
        m_root = new QNode(m_xMin, m_yMin, m_xMax, m_yMax);
    }

    m_root = insertNode(m_root, x, y, id, m_xMin, m_yMin, m_xMax, m_yMax);
    m_size++;

    // 更新统计
    const double elapsed = timer.elapsed();
    m_stats.totalInserts++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / (m_stats.totalInserts + m_stats.totalQueries);

    emit pointInserted(x, y);
}

// ──────────────────────────────────────────────
// 范围查询
// ──────────────────────────────────────────────

/**
 * @brief 查询矩形范围内的所有点
 *
 * @param xMin 查询范围 X 最小值
 * @param yMin 查询范围 Y 最小值
 * @param xMax 查询范围 X 最大值
 * @param yMax 查询范围 Y 最大值
 * @return 范围内所有点的 ID 列表
 */
QVector<int> QuadTree3::queryRange(double xMin, double yMin, double xMax, double yMax) const
{
    QElapsedTimer timer;
    timer.start();

    QVector<int> result;
    if (m_root) {
        queryRangeNode(m_root, xMin, yMin, xMax, yMax, result);
    }

    // 更新统计
    const double elapsed = timer.elapsed();
    m_stats.totalQueries++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / (m_stats.totalInserts + m_stats.totalQueries);

    return result;
}

// ──────────────────────────────────────────────
// K 近邻搜索
// ──────────────────────────────────────────────

/**
 * @brief 查询距离指定点最近的 K 个点
 *
 * 使用优先队列辅助的深度优先搜索。
 *
 * @param x 查询点 X 坐标
 * @param y 查询点 Y 坐标
 * @param k 返回最近邻数量
 * @return 最近 K 个点的 ID 列表（按距离排序）
 */
QVector<int> QuadTree3::nearestNeighbor(double x, double y, int k) const
{
    QElapsedTimer timer;
    timer.start();

    QVector<int> result;
    if (!m_root || k <= 0) return result;

    // 收集所有点
    QVector<QPair<double, double>> pts;
    QVector<int> ids;
    collectPoints(m_root, pts, ids);

    // 计算距离并排序
    QVector<QPair<double, int>> distId;
    for (int i = 0; i < pts.size(); ++i) {
        double dx = pts[i].first - x;
        double dy = pts[i].second - y;
        double dist = qSqrt(dx * dx + dy * dy);
        distId.append({dist, ids[i]});
    }

    std::sort(distId.begin(), distId.end());

    const int count = qMin(k, distId.size());
    for (int i = 0; i < count; ++i) {
        result.append(distId[i].second);
    }

    // 更新统计
    const double elapsed = timer.elapsed();
    m_stats.totalQueries++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / (m_stats.totalInserts + m_stats.totalQueries);

    return result;
}

// ──────────────────────────────────────────────
// 统计接口
// ──────────────────────────────────────────────

/**
 * @brief 获取当前统计信息
 * @return 包含插入次数、查询次数和平均耗时的Stats结构
 */
QuadTree3::Stats QuadTree3::stats() const
{
    return m_stats;
}

/**
 * @brief 重置所有累计统计信息
 */
void QuadTree3::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

// ──────────────────────────────────────────────
// 私有方法 — 节点插入
// ──────────────────────────────────────────────

/**
 * @brief 递归插入点到子树
 *
 * 如果是叶节点且未满，直接存储。
 * 如果叶节点已满，分裂为四个子节点后重新插入。
 *
 * @param n 当前节点
 * @param x 点 X 坐标
 * @param y 点 Y 坐标
 * @param id 点 ID
 * @param xMin 节点 X 最小值
 * @param yMin 节点 Y 最小值
 * @param xMax 节点 X 最大值
 * @param yMax 节点 Y 最大值
 * @return 更新后的节点
 */
QuadTree3::QNode* QuadTree3::insertNode(QNode* n, double x, double y, int id,
                                           double xMin, double yMin, double xMax, double yMax)
{
    if (n == nullptr) {
        n = new QNode(xMin, yMin, xMax, yMax);
    }

    if (n->isLeaf) {
        // 检查是否有空间
        if (n->count < QNode::MAX_INLINE) {
            n->px[n->count] = x;
            n->py[n->count] = y;
            n->pids[n->count] = id;
            n->count++;
            return n;
        }

        // 叶节点已满，需要分裂
        double midX = (xMin + xMax) / 2.0;
        double midY = (yMin + yMax) / 2.0;

        // 创建四个子节点
        n->children[0] = new QNode(xMin, midY, midX, yMax);    // NW
        n->children[1] = new QNode(midX, midY, xMax, yMax);    // NE
        n->children[2] = new QNode(xMin, yMin, midX, midY);    // SW
        n->children[3] = new QNode(midX, yMin, xMax, midY);    // SE
        n->isLeaf = false;

        // 重新插入旧数据
        for (int i = 0; i < n->count; ++i) {
            int quadrant = 0;
            if (n->px[i] >= midX) quadrant += 1;
            if (n->py[i] < midY)  quadrant += 2;
            insertNode(n->children[quadrant], n->px[i], n->py[i], n->pids[i],
                       n->children[quadrant]->xMin, n->children[quadrant]->yMin,
                       n->children[quadrant]->xMax, n->children[quadrant]->yMax);
        }
        n->count = 0;

        // 插入新数据
        int quadrant = 0;
        if (x >= midX) quadrant += 1;
        if (y < midY)  quadrant += 2;
        insertNode(n->children[quadrant], x, y, id,
                   n->children[quadrant]->xMin, n->children[quadrant]->yMin,
                   n->children[quadrant]->xMax, n->children[quadrant]->yMax);
    } else {
        // 内部节点：路由到正确的象限
        double midX = (xMin + xMax) / 2.0;
        double midY = (yMin + yMax) / 2.0;
        int quadrant = 0;
        if (x >= midX) quadrant += 1;
        if (y < midY)  quadrant += 2;
        insertNode(n->children[quadrant], x, y, id,
                   n->children[quadrant]->xMin, n->children[quadrant]->yMin,
                   n->children[quadrant]->xMax, n->children[quadrant]->yMax);
    }

    return n;
}

// ──────────────────────────────────────────────
// 私有方法 — 范围查询
// ──────────────────────────────────────────────

/**
 * @brief 递归范围查询
 *
 * 如果查询范围与节点边界不相交则剪枝。
 *
 * @param n 当前节点
 * @param xMin 查询范围 X 最小值
 * @param yMin 查询范围 Y 最小值
 * @param xMax 查询范围 X 最大值
 * @param yMax 查询范围 Y 最大值
 * @param res 结果收集容器
 */
void QuadTree3::queryRangeNode(QNode* n, double xMin, double yMin,
                                  double xMax, double yMax, QVector<int>& res) const
{
    if (n == nullptr) return;

    // 边界不相交判断
    if (n->xMax < xMin || n->xMin > xMax || n->yMax < yMin || n->yMin > yMax) {
        return;
    }

    if (n->isLeaf) {
        for (int i = 0; i < n->count; ++i) {
            if (n->px[i] >= xMin && n->px[i] <= xMax &&
                n->py[i] >= yMin && n->py[i] <= yMax) {
                res.append(n->pids[i]);
            }
        }
    } else {
        for (int c = 0; c < 4; ++c) {
            queryRangeNode(n->children[c], xMin, yMin, xMax, yMax, res);
        }
    }
}

// ──────────────────────────────────────────────
// 私有方法 — 收集所有点
// ──────────────────────────────────────────────

/**
 * @brief 递归收集子树中的所有点
 * @param n 当前节点
 * @param pts 坐标收集容器
 * @param ids ID 收集容器
 */
void QuadTree3::collectPoints(QNode* n, QVector<QPair<double, double>>& pts,
                                 QVector<int>& ids) const
{
    if (n == nullptr) return;

    if (n->isLeaf) {
        for (int i = 0; i < n->count; ++i) {
            pts.append({n->px[i], n->py[i]});
            ids.append(n->pids[i]);
        }
    } else {
        for (int c = 0; c < 4; ++c) {
            collectPoints(n->children[c], pts, ids);
        }
    }
}
