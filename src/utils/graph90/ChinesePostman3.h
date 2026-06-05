#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief ChinesePostman3 - 中国邮递员问题求解器
 *
 * 求解经过图中所有边至少一次的最短闭合路径，
 * 使用最小权完美匹配处理奇度顶点。
 */
class ChinesePostman3 : public QObject {
    Q_OBJECT
public:
    struct Stats {
        int totalToursComputed = 0;
        int totalEdges = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit ChinesePostman3(QObject* parent = nullptr);

    /** @brief 设置带权无向图 */
    void setGraph(const QVector<QVector<QPair<int,double>>>& adjList);

    /** @brief 求解最优邮递员路径(顶点序列) */
    QVector<int> solve();

    /** @brief 获取路径总权重 */
    double tourWeight() const;

    /** @brief 检查图是否为欧拉图(所有顶点度为偶数) */
    bool isEulerian() const;

    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    void tourComputed(int edgeCount, double totalWeight);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
    double m_tourWeight = 0.0;
    QVector<QVector<QPair<int,double>>> m_adjList;
};
