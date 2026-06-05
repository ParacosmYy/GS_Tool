#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief SteinerTree4 - 斯坦纳树求解器
 *
 * 求解图上的斯坦纳树问题，连接指定终端节点集合，
 * 使用近似算法(2-近似)求解NP-hard问题。
 */
class SteinerTree4 : public QObject {
    Q_OBJECT
public:
    struct Stats {
        int totalTreesComputed = 0;
        int totalSteinerNodes = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit SteinerTree4(QObject* parent = nullptr);

    /** @brief 设置边权重的邻接表 */
    void setGraph(const QVector<QVector<QPair<int,double>>>& adjList);

    /** @brief 指定终端节点，求解斯坦纳树 */
    QVector<QPair<int,int>> solve(const QVector<int>& terminals);

    /** @brief 获取斯坦纳树的总权重 */
    double totalWeight() const;

    /** @brief 获取斯坦纳(非终端)节点 */
    QVector<int> steinerNodes() const;

    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    void treeComputed(int edgeCount, double weight);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
    double m_totalWeight = 0.0;
    QVector<QVector<QPair<int,double>>> m_adjList;
};
