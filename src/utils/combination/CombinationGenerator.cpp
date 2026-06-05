/**
 * @file CombinationGenerator.cpp
 * @brief 组合生成器实现 — 字典序/排名/逆排名
 */

#include "utils/combination/CombinationGenerator.h"

#include <QElapsedTimer>
#include <algorithm>

/** @brief 构造函数 @param parent 父对象 */
CombinationGenerator::CombinationGenerator(QObject* parent)
    : QObject(parent), m_timeSum(0.0) {}

/** @brief 下一个组合(字典序) @param comb 组合 @param n 总数 @return 是否成功 */
bool CombinationGenerator::nextCombination(QVector<int>& comb, int n)
{
    int k = comb.size();
    if (k == 0) return false;

    /* 从右向左找第一个可以递增的位置 */
    int i = k - 1;
    while (i >= 0 && comb[i] >= n - k + i) --i;

    if (i < 0) return false; /* 已经是最后一个组合 */

    comb[i]++;
    for (int j = i + 1; j < k; ++j) {
        comb[j] = comb[j - 1] + 1;
    }

    m_stats.totalGenerated++;
    m_stats.totalOperations++;
    return true;
}

/** @brief 生成所有组合 @param n 总元素数 @param k 选取数 @return 所有组合 */
QVector<QVector<int>> CombinationGenerator::allCombinations(int n, int k)
{
    QElapsedTimer timer;
    timer.start();

    QVector<QVector<int>> result;
    if (k < 0 || k > n || n < 0) return result;

    if (k == 0) {
        result.append(QVector<int>());
        emit combinationsGenerated(1);
        return result;
    }

    /* 初始组合: {0,1,2,...,k-1} */
    QVector<int> comb(k);
    for (int i = 0; i < k; ++i) comb[i] = i;

    result.append(QVector<int>(comb));
    while (nextCombination(comb, n)) {
        result.append(QVector<int>(comb));
    }

    m_stats.totalGenerated += result.size();
    m_stats.totalOperations++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs =
        (m_stats.totalOperations > 0) ? m_timeSum / m_stats.totalOperations : 0.0;

    emit combinationsGenerated(result.size());
    return result;
}

/** @brief 组合排名 @param comb 组合 @param n 总数 @return 字典序排名 */
quint64 CombinationGenerator::rank(const QVector<int>& comb, int n)
{
    int k = comb.size();
    if (k == 0) return 0;

    quint64 r = 0;
    int prev = -1;

    for (int i = 0; i < k; ++i) {
        /* 对每个位置，累加前面跳过的组合数 */
        for (int v = prev + 1; v < comb[i]; ++v) {
            r += binomial(n - 1 - v, k - 1 - i);
        }
        prev = comb[i];
    }

    m_stats.totalOperations++;
    return r;
}

/** @brief 逆排名 @param rankValue 排名 @param n 总数 @param k 选取数 @return 组合 */
QVector<int> CombinationGenerator::unrank(quint64 rankValue, int n, int k)
{
    if (k == 0) return {};
    if (k < 0 || k > n) return {};

    QVector<int> comb(k);
    int prev = -1;
    quint64 r = rankValue;

    for (int i = 0; i < k; ++i) {
        /* 寻找第i个元素的值 */
        for (int v = prev + 1; v <= n - (k - i); ++v) {
            quint64 cnt = binomial(n - 1 - v, k - 1 - i);
            if (r < cnt) {
                comb[i] = v;
                prev = v;
                break;
            }
            r -= cnt;
        }
    }

    m_stats.totalOperations++;
    return comb;
}

/** @brief 组合数C(n,k) @param n 总数 @param k 选取数 @return 组合数 */
quint64 CombinationGenerator::binomial(int n, int k)
{
    if (k < 0 || k > n) return 0;
    if (k == 0 || k == n) return 1;

    /* 使用较小的k值 */
    if (k > n - k) k = n - k;

    quint64 result = 1;
    for (int i = 0; i < k; ++i) {
        result *= static_cast<quint64>(n - i);
        result /= static_cast<quint64>(i + 1);
    }
    return result;
}

/** @brief 重置统计 */
void CombinationGenerator::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
