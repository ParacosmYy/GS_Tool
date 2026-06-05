/**
 * @file RTreeSearch.h
 * @brief R树空间索引 — 矩形对象的插入与范围查询
 *
 * 功能: 实现R树空间索引结构，支持矩形区域的插入和相交查询。
 *       适用于嵌入式可视化中的碰撞检测、空间查询和图层管理。
 *
 * 协作: QuadtreeRegion(区域四叉树) / OctreeVolume(三维空间索引) / KdTreeBalancer(点索引)
 */
#ifndef RTREESEARCH_H
#define RTREESEARCH_H

#include <QObject>
#include <QVector>

/**
 * @brief R树空间索引
 *
 * 支持矩形数据的高效插入和范围查询。
 * 内部使用MBR(最小包围矩形)进行层次化空间索引。
 */
class RTreeSearch : public QObject {
    Q_OBJECT

public:
    /** @brief 统计信息 */
    struct Stats {
        quint64 totalInserts = 0;      ///< 累计插入次数
        quint64 totalSearches = 0;     ///< 累计查询次数
        double avgProcessingTimeMs = 0.0; ///< 平均处理时间(ms)
    };

    explicit RTreeSearch(int maxEntries = 8, QObject* parent = nullptr);
    ~RTreeSearch();

    /**
     * @brief 插入矩形数据
     * @param xMin 最小X坐标
     * @param yMin 最小Y坐标
     * @param xMax 最大X坐标
     * @param yMax 最大Y坐标
     * @param data 关联数据ID
     */
    void insert(double xMin, double yMin, double xMax, double yMax, int data);

    /**
     * @brief 范围查询，返回与查询矩形相交的所有数据ID
     * @param xMin 查询矩形最小X
     * @param yMin 查询矩形最小Y
     * @param xMax 查询矩形最大X
     * @param yMax 查询矩形最大Y
     * @return 匹配的数据ID列表
     */
    QVector<int> search(double xMin, double yMin, double xMax, double yMax);

    /** @brief 获取统计信息 */
    const Stats& stats() const { return m_stats; }

    /** @brief 重置统计信息 */
    void resetStatistics();

signals:
    /** @brief 搜索完成信号 @param resultCount 结果数量 */
    void searchCompleted(int resultCount);

private:
    /** @brief 轴对齐包围盒 */
    struct BBox {
        double xMin, yMin, xMax, yMax; ///< 边界坐标
        int data;                       ///< 关联数据
    };

    /** @brief R树节点 */
    struct Node {
        double xMin, yMin, xMax, yMax;  ///< 节点MBR
        QVector<BBox> entries;          ///< 叶子节点条目
        QVector<Node*> children;        ///< 内部节点子节点
        bool isLeaf;                     ///< 是否为叶子节点
    };

    Node* createNode(bool isLeaf);
    void destroyNode(Node* node);
    void updateMBR(Node* node);
    Node* chooseSubtree(Node* node, double xMin, double yMin,
                        double xMax, double yMax);
    void searchNode(Node* node, double xMin, double yMin,
                    double xMax, double yMax, QVector<int>& result);
    double enlargement(const Node* node, double xMin, double yMin,
                       double xMax, double yMax) const;

    int m_maxEntries;   ///< 每个节点最大条目数
    Node* m_root;       ///< 根节点
    Stats m_stats;      ///< 统计信息
    double m_timeSum;    ///< 处理时间累加器
};

#endif // RTREESEARCH_H
