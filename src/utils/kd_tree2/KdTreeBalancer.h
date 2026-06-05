/**
 * @file KdTreeBalancer.h
 * @brief 平衡KD树 — 基于中位数分割的最近邻搜索
 *
 * 功能: 构建平衡KD树并支持最近邻查询。使用std::nth_element
 *       进行O(n)中位数查找，保证树高度O(log n)。
 *       适用于嵌入式数据可视化中的二维点索引和最近邻搜索。
 *
 * 协作: KDTree(通用k维) / RTreeSearch(矩形索引) / ApproxNearestNeighbor(近似查询)
 */
#ifndef KDTREEBALANCER_H
#define KDTREEBALANCER_H

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 平衡KD树构建器
 *
 * 专用于二维点的平衡KD树，支持建树和最近邻查询。
 * 通过中位数分割保证树的平衡性。
 */
class KdTreeBalancer : public QObject {
    Q_OBJECT

public:
    /** @brief 统计信息 */
    struct Stats {
        quint64 totalBuilds = 0;       ///< 累计建树次数
        quint64 totalQueries = 0;      ///< 累计查询次数
        double avgProcessingTimeMs = 0.0; ///< 平均处理时间(ms)
    };

    explicit KdTreeBalancer(QObject* parent = nullptr);
    ~KdTreeBalancer();

    /**
     * @brief 从点集构建平衡KD树
     * @param points 二维点集(x, y)
     */
    void build(const QVector<QPair<double, double>>& points);

    /**
     * @brief 查询最近邻
     * @param x 查询点X坐标
     * @param y 查询点Y坐标
     * @return (最近点索引, 距离)，树为空时返回(-1, -1)
     */
    QPair<int, double> nearestNeighbor(double x, double y);

    /** @brief 获取统计信息 */
    const Stats& stats() const { return m_stats; }

    /** @brief 重置统计信息 */
    void resetStatistics();

signals:
    /** @brief 查询完成信号 @param distance 到最近邻的距离 */
    void queryCompleted(double distance);

private:
    /** @brief 内部点结构 */
    struct Point2D {
        double x, y; ///< 坐标
        int origIndex; ///< 原始索引
    };

    /** @brief KD树节点 */
    struct Node {
        Point2D point;          ///< 数据点
        Node* left = nullptr;   ///< 左子树
        Node* right = nullptr;  ///< 右子树
    };

    Node* buildNode(QVector<Point2D>& points, int start, int end, int depth);
    void nearestNode(Node* node, double x, double y, int depth,
                     int& bestIdx, double& bestDist);

    void destroyTree(Node* node);

    Node* m_root;       ///< 根节点
    int m_size;          ///< 节点计数
    Stats m_stats;       ///< 统计信息
    double m_timeSum;    ///< 处理时间累加器
};

#endif // KDTREEBALANCER_H
