/**
 * @file VotingCompositor.h
 * @brief 投票组合器 — 多模型集成决策
 *
 * 功能: 支持多数投票/加权投票/软投票三种集成策略，
 *       统计投票次数/一致性比例/加权准确率。
 */
#ifndef VOTINGCOMPOSITOR_H
#define VOTINGCOMPOSITOR_H

#include <QObject>
#include <QMap>
#include <QVector>
#include <QList>

/**
 * @class VotingCompositor
 * @brief 多模型投票集成，综合多个预测结果
 */
class VotingCompositor : public QObject {
    Q_OBJECT
public:
    /** 投票策略 */
    enum class Strategy {
        Majority,       ///< 多数投票
        Weighted,       ///< 加权投票
        SoftVoting      ///< 软投票(概率平均)
    };

    /** 投票统计 */
    struct Stats {
        quint64 totalVotes = 0;
        double  avgAgreement = 0.0;     ///< 平均一致性
        quint64 unanimousCount = 0;     ///< 全部一致次数
        double  averageProcessingTimeMs = 0.0;
    };

    explicit VotingCompositor(QObject* parent = nullptr);

    void setStrategy(Strategy s);
    void setModelWeight(int modelId, double weight);

    /** 硬投票: 每个模型给出一个类别 */
    int vote(const QVector<int>& predictions);

    /** 软投票: 每个模型给出概率分布 */
    int softVote(const QVector<QMap<int, double>>& probaDistributions);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void voteComplete(int result, double agreement);

private:
    Strategy m_strategy;
    QMap<int, double> m_weights;
    Stats m_stats;
    double m_timeSum;
};

#endif // VOTINGCOMPOSITOR_H
