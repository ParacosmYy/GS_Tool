/**
 * @file CombinationGenerator.h
 * @brief 组合生成器 — n选k/next_combination/排名逆排名
 *
 * 功能: C(n,k)组合生成，next_combination字典序迭代，
 *       组合排名(rank)与逆排名(unrank)，
 *       统计生成次数/组合数/耗时。
 */
#ifndef COMBINATIONGENERATOR_H
#define COMBINATIONGENERATOR_H

#include <QObject>
#include <QVector>

/**
 * @brief 组合生成器
 */
class CombinationGenerator : public QObject {
    Q_OBJECT

public:
    /** @brief 统计 */
    struct Stats {
        quint64 totalGenerated = 0;   ///< 累计生成组合数
        quint64 totalOperations = 0;  ///< 累计操作次数
        double  avgProcessingTimeMs = 0.0; ///< 平均处理时间
    };

    explicit CombinationGenerator(QObject* parent = nullptr);

    /** @brief 下一个组合(字典序) @param comb 当前组合(会被修改) @param n 总元素数 @return 是否有下一个组合 */
    bool nextCombination(QVector<int>& comb, int n);

    /** @brief 生成所有组合 @param n 总元素数 @param k 选取数 @return 所有组合 */
    QVector<QVector<int>> allCombinations(int n, int k);

    /** @brief 组合排名 @param comb 组合 @param n 总元素数 @return 字典序排名 */
    quint64 rank(const QVector<int>& comb, int n);

    /** @brief 逆排名 @param rankValue 排名 @param n 总元素数 @param k 选取数 @return 组合 */
    QVector<int> unrank(quint64 rankValue, int n, int k);

    /** @brief 组合数C(n,k) @param n 总数 @param k 选取数 @return 组合数 */
    static quint64 binomial(int n, int k);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 组合生成 @param count 本次生成数量 */
    void combinationsGenerated(int count);

private:
    Stats m_stats;
    double m_timeSum;
};

#endif // COMBINATIONGENERATOR_H
