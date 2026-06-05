/**
 * @file KDTree.h
 * @brief KD树 — k维空间二叉搜索树
 *
 * 功能: 支持k维点的最近邻查询、范围查询和半径查询。
 *       用于嵌入式数据可视化和空间索引。
 *
 * 协作: QuadTree(2D空间) / KMeansClusterer(聚类) / PcaAnalyzer(降维)
 */
#ifndef KDTREE_H
#define KDTREE_H

#include <QObject>
#include <QVector>

/**
 * @brief KD树 — k维空间索引
 */
class KDTree : public QObject {
    Q_OBJECT

public:
    /** @brief k维点 */
    using Point = QVector<double>;

    /** @brief 统计信息 */
    struct Stats {
        quint64 totalInserts = 0;       ///< 累计插入次数
        quint64 totalQueries = 0;       ///< 累计查询次数
        double avgProcessingTimeMs = 0.0; ///< 平均处理时间(ms)
    };

    explicit KDTree(int dimensions, QObject* parent = nullptr);
    ~KDTree();

    /** @brief 插入点 @param point k维点 */
    void insert(const Point& point);

    /** @brief 批量建树(比逐点插入快)
     *  @param points 点集 */
    void build(const QVector<Point>& points);

    /** @brief 最近邻查询
     *  @param target 查询点
     *  @return 最近点，空表示树为空 */
    Point nearestNeighbor(const Point& target) const;

    /** @brief K近邻查询
     *  @param target 查询点
     *  @param k 邻居数
     *  @return 按距离排序的k个最近点 */
    QVector<Point> kNearestNeighbors(const Point& target, int k) const;

    /** @brief 范围查询(矩形)
     *  @param minCorner 最小角
     *  @param maxCorner 最大角
     *  @return 范围内的点 */
    QVector<Point> rangeQuery(const Point& minCorner,
                              const Point& maxCorner) const;

    /** @brief 节点数 */
    int size() const { return m_size; }

    /** @brief 获取统计 */
    const Stats& stats() const { return m_stats; }

    /** @brief 重置统计 */
    void resetStatistics();

signals:
    /** @brief 插入完成 @param size 当前节点数 */
    void insertCompleted(int size);

    /** @brief 查询完成 @param resultCount 结果数 */
    void queryCompleted(int resultCount);

private:
    /** @brief KD树节点 */
    struct Node {
        Point point;           ///< 数据点
        Node* left = nullptr;  ///< 左子树
        Node* right = nullptr; ///< 右子树
    };

    /** @brief 距离平方 */
    double distanceSquared(const Point& a, const Point& b) const;

    /** @brief 递归插入 */
    Node* insertNode(Node* node, const Point& point, int depth);

    /** @brief 递归建树 */
    Node* buildNode(QVector<Point>& points, int start, int end, int depth);

    /** @brief 递归最近邻 */
    void nearestNode(Node* node, const Point& target, int depth,
                     Node*& best, double& bestDist) const;

    /** @brief 递归范围查询 */
    void rangeNode(Node* node, const Point& minC, const Point& maxC,
                   int depth, QVector<Point>& result) const;

    /** @brief 递归销毁 */
    void destroyTree(Node* node);

    int m_dimensions;   ///< 维度
    Node* m_root;       ///< 根节点
    int m_size;         ///< 节点计数
    double m_timeSum;   ///< 处理时间累加器
    Stats m_stats;      ///< 统计信息
};

#endif // KDTREE_H
