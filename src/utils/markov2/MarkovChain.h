/**
 * @file MarkovChain.h
 * @brief 马尔可夫链 — 状态转移概率模型
 *
 * 功能: 一阶马尔可夫链，支持状态转移矩阵训练、下一状态预测、
 *       平稳分布计算，统计预测次数/准确率。
 */
#ifndef MARKOVCHAIN2_H
#define MARKOVCHAIN2_H

#include <QObject>
#include <QMap>
#include <QVector>
#include <QPair>

/**
 * @class MarkovChain
 * @brief 一阶马尔可夫链模型
 */
class MarkovChain : public QObject {
    Q_OBJECT
public:
    /** 链统计 */
    struct Stats {
        quint64 totalPredictions = 0;
        quint64 correctPredictions = 0;
        double  accuracy = 0.0;
        int     stateCount = 0;
        double  averageProcessingTimeMs = 0.0;
    };

    explicit MarkovChain(QObject* parent = nullptr);

    /** 训练: 观察序列 */
    void train(const QVector<int>& sequence);

    /** 预测下一状态 */
    int predict(int currentState);

    /** 预测概率分布 */
    QMap<int, double> predictProba(int currentState);

    /** 计算平稳分布 */
    QMap<int, double> stationaryDistribution() const;

    /** 生成序列 */
    QVector<int> generate(int startState, int length);

    /** 查询 */
    int stateCount() const;
    double transitionProb(int from, int to) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();
    void reset();

signals:
    void predictionMade(int fromState, int toState, double probability);

private:
    QMap<int, QMap<int, int>> m_transitions;   ///< from → (to → count)
    QMap<int, int> m_stateCounts;               ///< state → total outgoing
    Stats m_stats;
    double m_timeSum;
};

#endif // MARKOVCHAIN2_H
