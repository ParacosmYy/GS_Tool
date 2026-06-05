#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief QuadTree4 - 四叉树空间索引
 *
 * 支持动态插入和范围查询的二维空间四叉树，
 * 自动细分超过容量的节点，适用于空间碰撞检测。
 */
class QuadTree4 : public QObject {
    Q_OBJECT
public:
    struct Stats {
        int totalPointsInserted = 0;
        int totalRangeQueries = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit QuadTree4(QObject* parent = nullptr);

    /** @brief 设置边界矩形和节点容量 */
    void setBounds(double xMin, double yMin, double xMax, double yMax, int capacity = 4);

    /** @brief 插入二维点 */
    bool insert(double x, double y, int data = 0);

    /** @brief 查询矩形范围内的所有点 */
    QVector<QPair<QPair<double,double>, int>> queryRange(
        double xMin, double yMin, double xMax, double yMax) const;

    /** @brief 获取树中的点总数 */
    int count() const;

    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    void nodeSubdivided(int depth);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
    struct QuadNode;
    QuadNode* m_root = nullptr;
};
