/**
 * @file OctreeVolume.h
 * @brief 八叉树 — 三维空间索引与范围查询
 *
 * 功能: 实现八叉树数据结构，支持三维空间点的插入和范围查询。
 *       适用于嵌入式3D数据可视化中的碰撞检测、体素管理和空间分区。
 *
 * 协作: QuadtreeRegion(二维扩展) / RTreeSearch(矩形索引) / KdTreeBalancer(点索引)
 */
#ifndef OCTREEVOLUME_H
#define OCTREEVOLUME_H

#include <QObject>
#include <QVector>

/**
 * @brief 八叉树三维空间索引
 *
 * 将三维空间递归分割为八个子立方体，
 * 支持空间点的插入和轴对齐包围盒查询。
 */
class OctreeVolume : public QObject {
    Q_OBJECT

public:
    /** @brief 统计信息 */
    struct Stats {
        quint64 totalInserts = 0;      ///< 累计插入次数
        quint64 totalQueries = 0;      ///< 累计查询次数
        double avgProcessingTimeMs = 0.0; ///< 平均处理时间(ms)
    };

    explicit OctreeVolume(int maxDepth = 8, QObject* parent = nullptr);
    ~OctreeVolume();

    /**
     * @brief 插入三维数据点
     * @param x X坐标
     * @param y Y坐标
     * @param z Z坐标
     * @param data 关联数据ID
     * @return 插入是否成功
     */
    bool insert(double x, double y, double z, int data);

    /**
     * @brief 三维范围查询
     * @param xMin 最小X
     * @param yMin 最小Y
     * @param zMin 最小Z
     * @param xMax 最大X
     * @param yMax 最大Y
     * @param zMax 最大Z
     * @return 范围内的数据ID列表
     */
    QVector<int> rangeQuery(double xMin, double yMin, double zMin,
                            double xMax, double yMax, double zMax);

    /** @brief 获取统计信息 */
    const Stats& stats() const { return m_stats; }

    /** @brief 重置统计信息 */
    void resetStatistics();

signals:
    /** @brief 查询完成信号 @param resultCount 结果数量 */
    void queryCompleted(int resultCount);

private:
    /** @brief 三维点数据 */
    struct Point3D {
        double x, y, z; ///< 坐标
        int data;        ///< 关联数据
    };

    /** @brief 八叉树节点 */
    struct OctNode {
        double cx, cy, cz;          ///< 节点中心坐标
        double halfSize;            ///< 节点半尺寸
        QVector<Point3D> points;    ///< 存储的点(叶子节点)
        OctNode* children[8];       ///< 八个子节点
        bool isLeaf;                ///< 是否为叶子节点
    };

    static constexpr int s_maxPointsPerNode = 16; ///< 叶子节点最大点数

    OctNode* createNode(double cx, double cy, double cz, double halfSize);
    bool insertNode(OctNode* node, double x, double y, double z, int data, int depth);
    void subdivide(OctNode* node);
    void queryNode(OctNode* node, double xMin, double yMin, double zMin,
                   double xMax, double yMax, double zMax, QVector<int>& result);
    void destroyNode(OctNode* node);

    int m_maxDepth;     ///< 最大深度
    OctNode* m_root;    ///< 根节点
    Stats m_stats;      ///< 统计信息
    double m_timeSum;   ///< 处理时间累加器
};

#endif // OCTREEVOLUME_H
