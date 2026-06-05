/**
 * @file PermutationGenerator.cpp
 * @brief 排列生成器实现 — 字典序/排名/逆排名
 */

#include "utils/permutation/PermutationGenerator.h"

#include <QElapsedTimer>
#include <algorithm>

/** @brief 构造函数 @param parent 父对象 */
PermutationGenerator::PermutationGenerator(QObject* parent)
    : QObject(parent), m_timeSum(0.0) {}

/** @brief 下一个排列(字典序) @param perm 排列 @return 是否成功 */
bool PermutationGenerator::nextPermutation(QVector<int>& perm)
{
    int n = perm.size();
    if (n < 2) return false;

    /* 从右向左找第一个升序位置 */
    int i = n - 2;
    while (i >= 0 && perm[i] >= perm[i + 1]) --i;

    if (i < 0) return false; /* 已经是最后一个排列 */

    /* 从右向左找第一个大于perm[i]的位置 */
    int j = n - 1;
    while (perm[j] <= perm[i]) --j;

    std::swap(perm[i], perm[j]);
    std::reverse(perm.begin() + i + 1, perm.end());

    m_stats.totalGenerated++;
    m_stats.totalOperations++;
    return true;
}

/** @brief 上一个排列(字典序) @param perm 排列 @return 是否成功 */
bool PermutationGenerator::prevPermutation(QVector<int>& perm)
{
    int n = perm.size();
    if (n < 2) return false;

    int i = n - 2;
    while (i >= 0 && perm[i] <= perm[i + 1]) --i;

    if (i < 0) return false;

    int j = n - 1;
    while (perm[j] >= perm[i]) --j;

    std::swap(perm[i], perm[j]);
    std::reverse(perm.begin() + i + 1, perm.end());

    m_stats.totalOperations++;
    return true;
}

/** @brief 生成所有全排列 @param n 元素数 @return 所有排列 */
QVector<QVector<int>> PermutationGenerator::allPermutations(int n)
{
    QElapsedTimer timer;
    timer.start();

    QVector<QVector<int>> result;
    if (n < 1 || n > 12) return result; /* 12! = 479M，限制 */

    QVector<int> perm(n);
    for (int i = 0; i < n; ++i) perm[i] = i;

    result.append(perm);
    while (nextPermutation(perm)) {
        result.append(perm);
    }

    m_stats.totalGenerated += result.size();
    m_stats.totalOperations++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs =
        (m_stats.totalOperations > 0) ? m_timeSum / m_stats.totalOperations : 0.0;

    emit permutationsGenerated(result.size());
    return result;
}

/** @brief 生成k-排列 @param n 总元素数 @param k 选取数 @return k-排列 */
QVector<QVector<int>> PermutationGenerator::kPermutations(int n, int k)
{
    QElapsedTimer timer;
    timer.start();

    QVector<QVector<int>> result;
    if (k < 1 || k > n || n > 15) return result;

    /* 使用递归生成k-排列 */
    QVector<bool> used(n, false);
    QVector<int> current;
    current.reserve(k);

    std::function<void()> generate = [&]() {
        if (current.size() == static_cast<size_t>(k)) {
            result.append(QVector<int>(current));
            return;
        }
        for (int i = 0; i < n; ++i) {
            if (!used[i]) {
                used[i] = true;
                current.append(i);
                generate();
                current.removeLast();
                used[i] = false;
            }
        }
    };

    generate();

    m_stats.totalGenerated += result.size();
    m_stats.totalOperations++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs =
        (m_stats.totalOperations > 0) ? m_timeSum / m_stats.totalOperations : 0.0;

    emit permutationsGenerated(result.size());
    return result;
}

/** @brief 排列排名 @param perm 排列 @return 字典序排名 */
quint64 PermutationGenerator::rank(const QVector<int>& perm)
{
    int n = perm.size();
    if (n == 0) return 0;

    QVector<bool> used(n, false);
    quint64 r = 0;

    for (int i = 0; i < n; ++i) {
        /* 计算比perm[i]小且未使用的元素数 */
        int count = 0;
        for (int j = 0; j < perm[i]; ++j) {
            if (!used[j]) ++count;
        }
        r += static_cast<quint64>(count) * factorial(n - 1 - i);
        used[perm[i]] = true;
    }

    m_stats.totalOperations++;
    return r;
}

/** @brief 逆排名 @param rankValue 排名 @param n 元素数 @return 排列 */
QVector<int> PermutationGenerator::unrank(quint64 rankValue, int n)
{
    if (n < 1) return {};

    QVector<int> perm(n);
    QVector<int> available(n);
    for (int i = 0; i < n; ++i) available[i] = i;

    quint64 r = rankValue;
    for (int i = 0; i < n; ++i) {
        quint64 f = factorial(n - 1 - i);
        quint64 idx = r / f;
        r = r % f;

        if (idx >= static_cast<quint64>(available.size())) {
            idx = available.size() - 1;
        }

        perm[i] = available[static_cast<int>(idx)];
        available.remove(static_cast<int>(idx));
    }

    m_stats.totalOperations++;
    return perm;
}

/** @brief 阶乘 @param n 非负整数 @return n! */
quint64 PermutationGenerator::factorial(int n)
{
    static QVector<quint64> table = {1, 1, 2, 6, 24, 120, 720, 5040, 40320,
                                      362880, 3628800, 39916800, 479001600,
                                      6227020800ULL, 87178291200ULL,
                                      1307674368000ULL, 20922789888000ULL,
                                      355687428096000ULL, 6402373705728000ULL,
                                      121645100408832000ULL, 2432902008176640000ULL};
    if (n < 0) return 0;
    if (n < table.size()) return table[n];
    quint64 result = table.last();
    for (int i = table.size(); i <= n; ++i) result *= i;
    return result;
}

/** @brief 排列数P(n,k) @param n 总数 @param k 选取数 @return 排列数 */
quint64 PermutationGenerator::permutationCount(int n, int k)
{
    if (k > n || k < 0) return 0;
    quint64 result = 1;
    for (int i = 0; i < k; ++i) {
        result *= static_cast<quint64>(n - i);
    }
    return result;
}

/** @brief 重置统计 */
void PermutationGenerator::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
