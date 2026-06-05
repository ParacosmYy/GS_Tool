#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief IntervalTree5 - 增强区间树
 *
 * 支持动态插入删除的增强区间树，基于红黑树实现，
 * 高效查询与给定区间重叠的所有区间。
 */
class IntervalTree5 : public QObject {
    Q_OBJECT
public:
    struct Stats {
        int totalIntervals = 0;
        int totalQueries = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit IntervalTree5(QObject* parent = nullptr);

    /** @brief 插入区间[low, high]并关联数据 */
    void insert(double low, double high, int data = 0);

    /** @brief 删除指定区间 */
    bool remove(double low, double high);

    /** @brief 查询与点value重叠的所有区间 */
    QVector<QPair<QPair<double,double>, int>> queryPoint(double value) const;

    /** @brief 查询与区间[low,high]重叠的所有区间 */
    QVector<QPair<QPair<double,double>, int>> queryInterval(double low, double high) const;

    /** @brief 获取树中区间总数 */
    int count() const;

    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    void intervalInserted(double low, double high);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
    struct Node;
    Node* m_root = nullptr;
};
