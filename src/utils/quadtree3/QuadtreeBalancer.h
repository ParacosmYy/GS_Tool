/**
 * @file QuadtreeBalancer.h
 * @brief 平衡四叉树 — 支持重平衡的空间索引
 *
 * 功能: 四叉树空间索引，支持点插入/范围查询/自动重平衡，
 *       统计插入/查询/重平衡次数/耗时。
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>
#include <QPointF>

class QuadtreeOptBalancer : public QObject {
    Q_OBJECT
public:
    /** 统计信息 */
    struct Stats {
        quint64 totalInserts = 0;      ///< 总插入次数
        quint64 totalQueries = 0;      ///< 总查询次数
        quint64 totalRebalances = 0;   ///< 总重平衡次数
        double  avgProcessingTimeMs = 0.0; ///< 平均处理时间(ms)
    };

    explicit QuadtreeOptBalancer(QObject* parent = nullptr);
    ~QuadtreeOptBalancer();

    /**
     * @brief 插入一个点
     * @param x X坐标
     * @param y Y坐标
     * @param value 关联值
     */
    void insert(double x, double y, double value);

    /**
     * @brief 范围查询
     * @param x1 矩形左下X
     * @param y1 矩形左下Y
     * @param x2 矩形右上X
     * @param y2 矩形右上Y
     * @return 范围内的点(坐标+值)
     */
    QVector<QPair<QPointF, double>> rangeQuery(
        double x1, double y1, double x2, double y2);

    /** @brief 重新平衡树(重建) */
    void rebalance();

    /** @brief 获取节点总数 */
    int nodeCount() const;

    /** @brief 获取统计信息 */
    const Stats& stats() const { return m_stats; }

    /** @brief 重置统计 */
    void resetStatistics();

signals:
    /** @brief 重平衡完成信号 @param nodes 重平衡后的节点数 */
    void rebalanced(int nodes);

private:
    /** 四叉树节点 */
    struct Node {
        double x, y;           ///< 点坐标
        double value;          ///< 关联值
        bool   occupied;       ///< 是否有数据
        Node*  children[4];    ///< 四个子节点(NW/NE/SW/SE)
        double bounds[4];      ///< 边界 [xmin, ymin, xmax, ymax]

        Node();
        ~Node();
    };

    /** @brief 递归插入 */
    void insertImpl(Node* node, double x, double y, double value);

    /** @brief 递归范围查询 */
    void rangeQueryImpl(Node* node, double x1, double y1,
                         double x2, double y2,
                         QVector<QPair<QPointF, double>>& result) const;

    /** @brief 收集所有点 */
    void collectPoints(Node* node,
                        QVector<QPair<QPointF, double>>& points) const;

    /** @brief 递归计算节点数 */
    int countNodes(Node* node) const;

    /** @brief 判断矩形是否与节点边界相交 */
    bool intersects(Node* node, double x1, double y1,
                    double x2, double y2) const;

    /** @brief 判断点是否在节点边界内 */
    bool contains(Node* node, double x, double y) const;

    /** @brief 获取子节点象限索引 */
    int getChildIndex(Node* node, double x, double y) const;

    /** @brief 删除子树 */
    void clearNode(Node* node);

    Node*  m_root;     ///< 根节点
    Stats  m_stats;
    double m_timeSum;
    int    m_size;     ///< 最大容量阈值
};
