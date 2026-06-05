#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 四叉树(Quad Tree)
 *
 * 二维空间分区数据结构，支持空间查询和碰撞检测。
 */
class QuadTree5 : public QObject {
    Q_OBJECT
public:
    struct Stats {
        int totalPointsInserted = 0;
        int totalRangeQueries = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit QuadTree5(double x, double y, double width, double height,
                        int capacity = 4, QObject* parent = nullptr);

    /** @brief 插入2D点 */
    bool insert(double x, double y, const QVariant& data = QVariant());

    /** @brief 范围查询 */
    QVector<QPair<QPair<double, double>, QVariant>> queryRange(double x, double y,
                                                                double w, double h) const;

    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    void rangeQueryCompleted(int resultCount);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
    int m_capacity = 4;
};
