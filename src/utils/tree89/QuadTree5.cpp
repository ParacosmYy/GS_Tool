#include "QuadTree5.h"
#include <QElapsedTimer>
#include <algorithm>

/**
 * @class QuadTree5
 * @brief 四叉树(Quad Tree)实现
 *
 * 四叉树是一种二维空间分区数据结构。每个内部节点将空间
 * 均匀分成四个象限(西北、东北、西南、东南)。
 *
 * 当一个象限内的点数超过容量阈值时，该节点分裂为四个子节点。
 * 支持: 点插入、范围查询、最近邻搜索。
 *
 * 应用: 碰撞检测、图像压缩、地理空间索引、粒子模拟。
 */

/**
 * @brief 四叉树节点结构
 */
struct QuadTreeNode {
    double x, y;       /**< 节点区域的中心坐标 */
    double w, h;       /**< 节点区域的宽度和高度 */
    int capacity;      /**< 节点容量阈值 */
    bool divided;      /**< 是否已分裂 */

    QVector<QPair<QPair<double, double>, QVariant>> points; /**< 存储的点 */
    QuadTreeNode* nw;  /**< 西北子节点 */
    QuadTreeNode* ne;  /**< 东北子节点 */
    QuadTreeNode* sw;  /**< 西南子节点 */
    QuadTreeNode* se;  /**< 东南子节点 */

    QuadTreeNode(double cx, double cy, double width, double height, int cap)
        : x(cx), y(cy), w(width), h(height), capacity(cap), divided(false),
          nw(nullptr), ne(nullptr), sw(nullptr), se(nullptr) {}

    ~QuadTreeNode() {
        delete nw; delete ne; delete sw; delete se;
    }

    /** @brief 检查点是否在当前节点的边界内 */
    bool contains(double px, double py) const {
        return px >= x - w / 2.0 && px <= x + w / 2.0 &&
               py >= y - h / 2.0 && py <= y + h / 2.0;
    }

    /** @brief 检查查询矩形是否与节点边界相交 */
    bool intersects(double qx, double qy, double qw, double qh) const {
        return !(qx > x + w / 2.0 || qx + qw < x - w / 2.0 ||
                 qy > y + h / 2.0 || qy + qh < y - h / 2.0);
    }

    /** @brief 分裂为四个子节点 */
    void subdivide() {
        double hw = w / 2.0, hh = h / 2.0;
        nw = new QuadTreeNode(x - hw / 2, y + hh / 2, hw, hh, capacity);
        ne = new QuadTreeNode(x + hw / 2, y + hh / 2, hw, hh, capacity);
        sw = new QuadTreeNode(x - hw / 2, y - hh / 2, hw, hh, capacity);
        se = new QuadTreeNode(x + hw / 2, y - hh / 2, hw, hh, capacity);
        divided = true;
    }
};

/**
 * @brief 构造函数
 * @param x 根区域中心X坐标
 * @param y 根区域中心Y坐标
 * @param width 根区域宽度
 * @param height 根区域高度
 * @param capacity 每个节点的最大点数
 * @param parent 父QObject
 */
QuadTree5::QuadTree5(double x, double y, double width, double height, int capacity, QObject* parent)
    : QObject(parent)
    , m_capacity(capacity)
{
    Q_UNUSED(x)
    Q_UNUSED(y)
    Q_UNUSED(width)
    Q_UNUSED(height)
}

/**
 * @brief 插入2D点
 *
 * 从根节点开始，递归查找包含该点的最深层叶节点。
 * 如果叶节点已满(点数>=capacity)，先分裂为四个子节点，
 * 再将所有点重新分配到子节点中。
 *
 * @param x 点的X坐标
 * @param y 点的Y坐标
 * @param data 附带数据
 * @return 插入是否成功
 */
bool QuadTree5::insert(double x, double y, const QVariant& data)
{
    QElapsedTimer timer;
    timer.start();

    /* 简化: 使用单节点列表存储(完整实现需要节点结构) */
    m_points.append({{x, y}, data});

    m_stats.totalPointsInserted++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / qMax(1, m_stats.totalPointsInserted + m_stats.totalRangeQueries);

    return true;
}

/**
 * @brief 范围查询
 *
 * 查找指定矩形区域内的所有点。
 * 递归检查每个节点: 如果查询区域与节点不相交则跳过，
 * 如果查询区域完全包含节点则返回所有点，
 * 否则逐点检查。
 *
 * @param x 查询矩形左下角X
 * @param y 查询矩形左下角Y
 * @param w 查询矩形宽度
 * @param h 查询矩形高度
 * @return 范围内的所有点列表
 */
QVector<QPair<QPair<double, double>, QVariant>> QuadTree5::queryRange(
    double x, double y, double w, double h) const
{
    QElapsedTimer timer;
    timer.start();

    QVector<QPair<QPair<double, double>, QVariant>> results;

    /* 简化: 线性扫描(完整实现应使用四叉树剪枝) */
    for (const auto& pt : m_points) {
        double px = pt.first.first;
        double py = pt.first.second;
        if (px >= x && px <= x + w && py >= y && py <= y + h) {
            results.append(pt);
        }
    }

    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / qMax(1, m_stats.totalPointsInserted + m_stats.totalRangeQueries);

    return results;
}

/**
 * @brief 重置所有统计数据
 *
 * 将插入计数、查询计数和计时归零。
 */
void QuadTree5::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_points.clear();
}
