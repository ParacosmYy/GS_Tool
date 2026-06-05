/**
 * @file QuadTree.h
 * @brief 四叉树 — 二维空间索引
 *
 * 功能: 四叉树空间索引，支持点插入/范围查询/最近邻搜索，
 *       统计操作次数/节点数/耗时。
 */
#ifndef QUADTREE_H
#define QUADTREE_H

#include <QObject>
#include <QVector>
#include <QPair>

class QuadTree : public QObject {
    Q_OBJECT
public:
    /** 二维点 */
    struct Point {
        double x, y;
        int data;
    };

    /** 矩形范围 */
    struct Rect {
        double x, y, w, h;
        bool contains(const Point& p) const {
            return p.x >= x && p.x < x + w && p.y >= y && p.y < y + h;
        }
        bool intersects(const Rect& r) const {
            return !(r.x >= x + w || r.x + r.w <= x || r.y >= y + h || r.y + r.h <= y);
        }
    };

    /** 统计 */
    struct Stats {
        quint64 totalInserts = 0;
        quint64 totalQueries = 0;
        int     totalNodes = 0;
        double  avgProcessingTimeMs = 0.0;
    };

    explicit QuadTree(const Rect& bounds, int capacity = 4,
                       QObject* parent = nullptr);
    ~QuadTree();

    /** @brief 插入点 @param point 点 */
    bool insert(const Point& point);

    /** @brief 范围查询 @param range 查询范围 @return 范围内的点 */
    QVector<Point> queryRange(const Rect& range) const;

    /** @brief 最近邻查询 @param x X坐标 @param y Y坐标 @param k 数量 @return 最近的k个点 */
    QVector<Point> nearestNeighbors(double x, double y, int k = 1) const;

    /** @brief 总点数(递归统计所有子节点) */
    int size() const;
    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void pointInserted(double x, double y);
    void rangeQueryCompleted(int resultCount);

private:
    void subdivide();
    void queryRangeImpl(const Rect& range, QVector<Point>& result) const;

    Rect m_bounds;
    int m_capacity;
    QVector<Point> m_points;
    bool m_divided;
    QuadTree* m_nw;
    QuadTree* m_ne;
    QuadTree* m_sw;
    QuadTree* m_se;
    int m_size;
    Stats m_stats;
    double m_timeSum;
};

#endif // QUADTREE_H
