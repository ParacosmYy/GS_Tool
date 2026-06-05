/**
 * @file PermutationGenerator.h
 * @brief 排列生成器 — 全排列/k-排列/排名逆排名
 *
 * 功能: next_permutation迭代，k-排列生成，
 *       排列排名(rank)与逆排名(unrank)，
 *       统计生成次数/排列数/耗时。
 */
#ifndef PERMUTATIONGENERATOR_H
#define PERMUTATIONGENERATOR_H

#include <QObject>
#include <QVector>

/**
 * @brief 排列生成器
 */
class PermutationGenerator : public QObject {
    Q_OBJECT

public:
    /** @brief 统计 */
    struct Stats {
        quint64 totalGenerated = 0;   ///< 累计生成排列数
        quint64 totalOperations = 0;  ///< 累计操作次数
        double  avgProcessingTimeMs = 0.0; ///< 平均处理时间
    };

    explicit PermutationGenerator(QObject* parent = nullptr);

    /** @brief 下一个排列(字典序) @param perm 当前排列(会被修改) @return 是否有下一个排列 */
    bool nextPermutation(QVector<int>& perm);

    /** @brief 上一个排列(字典序) @param perm 当前排列(会被修改) @return 是否有上一个排列 */
    bool prevPermutation(QVector<int>& perm);

    /** @brief 生成所有全排列 @param n 元素数 @return 所有排列 */
    QVector<QVector<int>> allPermutations(int n);

    /** @brief 生成k-排列 @param n 总元素数 @param k 选取数 @return k-排列列表 */
    QVector<QVector<int>> kPermutations(int n, int k);

    /** @brief 排列排名(rank) @param perm 排列 @return 字典序排名(从0开始) */
    quint64 rank(const QVector<int>& perm);

    /** @brief 逆排名(unrank) @param rankValue 排名 @param n 元素数 @return 对应排列 */
    QVector<int> unrank(quint64 rankValue, int n);

    /** @brief 阶乘 @param n 非负整数 @return n! */
    static quint64 factorial(int n);

    /** @brief 排列数P(n,k) @param n 总数 @param k 选取数 @return 排列数 */
    static quint64 permutationCount(int n, int k);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 排列生成 @param count 本次生成数量 */
    void permutationsGenerated(int count);

private:
    Stats m_stats;
    double m_timeSum;
};

#endif // PERMUTATIONGENERATOR_H
