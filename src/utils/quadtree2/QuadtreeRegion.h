/**
 * @file QuadtreeRegion.h
 * @brief 区域四叉树 — 网格/图像压缩与区域查询
 *
 * 功能: 实现区域四叉树，根据阈值递归分割网格实现自适应压缩。
 *       支持任意点的值查询，适用于嵌入式数据可视化中的
 *       图像压缩、地形LOD和热力图分层。
 *
 * 协作: GeodesicDistance(距离场) / OctreeVolume(三维扩展) / RTreeSearch(矩形索引)
 */
#ifndef QUADTREEREGION_H
#define QUADTREEREGION_H

#include <QObject>
#include <QVector>

/**
 * @brief 区域四叉树
 *
 * 将均匀网格递归分割为四个象限，每个叶子节点存储区域内的代表值。
 * 当区域内值的方差小于阈值时停止分割。
 */
class QuadtreeRegion : public QObject {
    Q_OBJECT

public:
    /** @brief 统计信息 */
    struct Stats {
        quint64 totalBuilds = 0;       ///< 累计建树次数
        quint64 totalQueries = 0;      ///< 累计查询次数
        double avgProcessingTimeMs = 0.0; ///< 平均处理时间(ms)
    };

    explicit QuadtreeRegion(QObject* parent = nullptr);
    ~QuadtreeRegion();

    /**
     * @brief 从网格构建区域四叉树
     * @param grid 二维数据网格
     * @param threshold 分割阈值(区域最大允许方差)
     */
    void build(const QVector<QVector<double>>& grid, double threshold);

    /**
     * @brief 查询指定位置的值
     * @param x 列坐标
     * @param y 行坐标
     * @return 该位置所属叶子节点的代表值，未建树返回0
     */
    double queryValue(int x, int y);

    /** @brief 获取统计信息 */
    const Stats& stats() const { return m_stats; }

    /** @brief 重置统计信息 */
    void resetStatistics();

signals:
    /** @brief 构建完成信号 @param nodeCount 总节点数 */
    void buildCompleted(int nodeCount);

private:
    /** @brief 四叉树节点 */
    struct QNode {
        int x, y;               ///< 区域左上角坐标
        int width, height;      ///< 区域尺寸
        double value;           ///< 代表值(叶子节点)
        QNode* children[4];     ///< 四个子节点: NW, NE, SW, SE
        bool isLeaf;            ///< 是否为叶子节点
    };

    QNode* buildNode(const QVector<QVector<double>>& grid,
                     int x, int y, int w, int h, double threshold);
    double queryNode(QNode* node, int x, int y);
    double regionVariance(const QVector<QVector<double>>& grid,
                          int x, int y, int w, int h, double& mean) const;
    void destroyNode(QNode* node);

    QNode* m_root;      ///< 根节点
    int m_nodeCount;     ///< 节点计数
    Stats m_stats;       ///< 统计信息
    double m_timeSum;    ///< 处理时间累加器
};

#endif // QUADTREEREGION_H
