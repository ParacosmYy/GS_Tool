/**
 * @file KdTreeNearest.h
 * @brief KD-Tree最近邻搜索 — KNN查询与球形范围查询
 *
 * 功能:
 *   - KD-Tree构建与平衡化
 *   - K最近邻(KNN)查询
 *   - 球形范围查询(Ball Query)
 *   - 支持任意维度(2D/3D/N-D)
 *   - 统计构建/查询次数、节点数、处理时间
 */

#pragma once

#include <QObject>
#include <QVector>
#include <QPair>
#include <cmath>

/**
 * @class KdTreeNearest
 * @brief KD-Tree空间索引 — 支持KNN与范围查询
 *
 * KD-Tree是k维空间中的二叉空间分割树，适合低维(2-20维)
 * 的最近邻搜索。构建O(n log n)，查询O(log n)平均。
 * 广泛应用于空间搜索、碰撞检测、点云处理等。
 */
class KdTreeNearest : public QObject
{
    Q_OBJECT

public:
    /** @brief 统计信息 */
    struct Stats {
        int totalBuilds = 0;          /**< 总构建次数 */
        int totalKNNQueries = 0;      /**< 总KNN查询次数 */
        int totalRangeQueries = 0;    /**< 总范围查询次数 */
        int totalNodesVisited = 0;    /**< 总访问节点数 */
        double avgProcessingTimeMs = 0.0; /**< 平均处理时间(ms) */
    };

    /** @brief 空间点: N维坐标 */
    using Point = QVector<double>;

    /** @brief 带距离的搜索结果 */
    struct Neighbor {
        int index;            /**< 点索引 */
        double distance;      /**< 距离(欧几里得) */
    };

    /**
     * @brief 构造函数
     * @param dimensions 空间维度
     * @param parent 父对象
     */
    explicit KdTreeNearest(int dimensions = 3, QObject* parent = nullptr);

    /** @brief 析构函数 */
    ~KdTreeNearest();

    /** @brief 禁止拷贝 */
    KdTreeNearest(const KdTreeNearest&) = delete;
    KdTreeNearest& operator=(const KdTreeNearest&) = delete;

    /**
     * @brief 构建KD-Tree
     * @param points 点集合(每个点维度必须匹配)
     * @return 是否成功构建
     */
    bool build(const QVector<Point>& points);

    /**
     * @brief K最近邻查询
     * @param query 查询点
     * @param k 最近邻数量
     * @return 按距离排序的邻居列表
     */
    QVector<Neighbor> knnSearch(const Point& query, int k);

    /**
     * @brief 球形范围查询
     * @param center 球心
     * @param radius 半径
     * @return 范围内的点索引及距离
     */
    QVector<Neighbor> rangeSearch(const Point& center, double radius);

    /**
     * @brief 查询最近邻(单点)
     * @param query 查询点
     * @return 最近邻(索引和距离)
     */
    Neighbor nearestNeighbor(const Point& query);

    /** @brief 树是否已构建 */
    bool isBuilt() const { return m_root != nullptr; }

    /** @brief 获取点数 */
    int pointCount() const { return m_points.size(); }

    /** @brief 获取维度 */
    int dimensions() const { return m_dimensions; }

    /** @brief 获取统计信息 */
    Stats stats() const;

    /** @brief 重置统计信息 */
    void resetStatistics();

signals:
    /** @brief 构建完成信号 */
    void buildCompleted(int pointCount, double elapsedMs);

    /** @brief 查询完成信号 */
    void queryCompleted(int resultCount, double elapsedMs);

private:
    /** @brief KD-Tree节点 */
    struct KdNode {
        int pointIndex;       /**< 点索引 */
        KdNode* left;         /**< 左子树 */
        KdNode* right;        /**< 右子树 */
        int splitDimension;   /**< 分割维度 */

        KdNode(int idx, int dim)
            : pointIndex(idx), left(nullptr), right(nullptr),
              splitDimension(dim) {}
    };

    /** @brief 递归构建 */
    KdNode* buildRecursive(QVector<int>& indices, int depth);

    /** @brief 递归KNN搜索 */
    void knnSearchRecursive(KdNode* node, const Point& query, int k,
                            QVector<Neighbor>& best) const;

    /** @brief 递归范围搜索 */
    void rangeSearchRecursive(KdNode* node, const Point& center,
                              double radius,
                              QVector<Neighbor>& results) const;

    /** @brief 递归释放 */
    void freeTree(KdNode* node);

    /** @brief 计算欧几里得距离 */
    double distance(const Point& a, const Point& b) const;

    /** @brief 维护KNN候选列表的最大堆性质 */
    void maintainMaxHeap(QVector<Neighbor>& heap, int k) const;

    KdNode* m_root;            /**< 根节点 */
    int m_dimensions;          /**< 空间维度 */
    QVector<Point> m_points;   /**< 点集合副本 */
    mutable int m_nodesVisited; /**< 单次查询访问节点计数 */
    mutable Stats m_stats;     /**< 统计信息 */
    mutable double m_timeSum = 0.0; /**< 累计时间 */
};
