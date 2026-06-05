/**
 * @file FisherExactTest.cpp
 * @brief Fisher精确检验实现 — 2×2列联表的精确概率计算
 */

#include "utils/fisher/FisherExactTest.h"

#include <QElapsedTimer>

#include <algorithm>

/** @brief 构造函数 @param parent 父对象 */
FisherExactTest::FisherExactTest(QObject* parent)
    : QObject(parent)
{
}

/** @brief 对数阶乘(避免溢出) @param n 非负整数 @return ln(n!) */
double FisherExactTest::logFactorial(qint64 n) const
{
    /* 使用Stirling近似 + 精确累积混合 */
    if (n <= 1) return 0.0;
    if (n <= 256) {
        double s = 0.0;
        for (qint64 i = 2; i <= n; ++i) {
            s += std::log(static_cast<double>(i));
        }
        return s;
    }
    /* Stirling近似: ln(n!) ≈ n*ln(n) - n + 0.5*ln(2πn) */
    double fn = static_cast<double>(n);
    return fn * std::log(fn) - fn + 0.5 * std::log(2.0 * M_PI * fn);
}

/** @brief 计算列联表超几何概率 @param table 列联表 @return 概率值 */
double FisherExactTest::tableProbability(const ContingencyTable& table) const
{
    qint64 rowSum1 = table.a + table.b;
    qint64 rowSum2 = table.c + table.d;
    qint64 colSum1 = table.a + table.c;
    qint64 colSum2 = table.b + table.d;
    qint64 n = rowSum1 + rowSum2;

    if (n == 0) return 1.0;

    /* 超几何分布: P = (a+b)!(c+d)!(a+c)!(b+d)! / (n! * a! * b! * c! * d!) */
    double logP = logFactorial(rowSum1) + logFactorial(rowSum2)
                + logFactorial(colSum1) + logFactorial(colSum2)
                - logFactorial(n)
                - logFactorial(table.a) - logFactorial(table.b)
                - logFactorial(table.c) - logFactorial(table.d);

    return std::exp(logP);
}

/** @brief 优势比 @param table 列联表 @return 优势比 */
double FisherExactTest::oddsRatio(const ContingencyTable& table) const
{
    /* 加0.5连续性校正避免除零 */
    double denom = static_cast<double>(table.b) * table.c;
    if (denom == 0.0) denom = 0.5;
    double numer = static_cast<double>(table.a) * table.d;
    if (numer == 0.0) numer = 0.5;
    return numer / denom;
}

/** @brief Fisher精确检验(双侧) @param table 列联表 @return 检验结果 */
FisherExactTest::TestResult FisherExactTest::test(const ContingencyTable& table)
{
    QElapsedTimer timer;
    timer.start();

    TestResult result;

    /* 原始表概率 */
    double pObs = tableProbability(table);
    result.oddsRatio = oddsRatio(table);
    result.logOddsRatio = std::log(result.oddsRatio);

    qint64 rowSum1 = table.a + table.b;
    qint64 rowSum2 = table.c + table.d;
    qint64 colSum1 = table.a + table.c;
    qint64 colSum2 = table.b + table.d;

    /* 遍历所有可能的a值，计算极端概率之和 */
    qint64 aMin = std::max(static_cast<qint64>(0), rowSum1 - colSum2);
    qint64 aMax = std::min(rowSum1, colSum1);

    double pLeft  = 0.0;
    double pRight = 0.0;
    double pTwo   = 0.0;

    for (qint64 a = aMin; a <= aMax; ++a) {
        ContingencyTable t;
        t.a = a;
        t.b = rowSum1 - a;
        t.c = colSum1 - a;
        t.d = rowSum2 - colSum1 + a;

        double p = tableProbability(t);

        if (a <= table.a) pLeft += p;
        if (a >= table.a) pRight += p;
        if (p <= pObs + 1e-12) pTwo += p;
    }

    result.pValueLeft     = std::min(pLeft, 1.0);
    result.pValueRight    = std::min(pRight, 1.0);
    result.pValueTwoSided = std::min(pTwo, 1.0);

    /* 更新统计 */
    double elapsed = timer.elapsed();
    ++m_stats.totalTests;
    m_stats.totalTablesComputed += static_cast<quint64>(aMax - aMin + 1);
    m_timeSumMs += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSumMs / m_stats.totalTests;

    if (result.pValueTwoSided < m_stats.minPValue) {
        m_stats.minPValue = result.pValueTwoSided;
    }
    if (result.pValueTwoSided > m_stats.maxPValue) {
        m_stats.maxPValue = result.pValueTwoSided;
    }

    emit testCompleted(result);
    return result;
}

/** @brief 单侧检验(右侧) @param table 列联表 @return 右侧p值 */
double FisherExactTest::testOneSided(const ContingencyTable& table)
{
    QElapsedTimer timer;
    timer.start();

    qint64 rowSum1 = table.a + table.b;
    qint64 rowSum2 = table.c + table.d;
    qint64 colSum1 = table.a + table.c;

    qint64 aMin = std::max(static_cast<qint64>(0), rowSum1 - (table.b + table.d));
    qint64 aMax = std::min(rowSum1, colSum1);

    double pRight = 0.0;
    for (qint64 a = table.a; a <= aMax; ++a) {
        ContingencyTable t;
        t.a = a;
        t.b = rowSum1 - a;
        t.c = colSum1 - a;
        t.d = rowSum2 - colSum1 + a;
        pRight += tableProbability(t);
    }

    double elapsed = timer.elapsed();
    ++m_stats.totalTests;
    m_timeSumMs += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSumMs / m_stats.totalTests;

    emit testCompleted(TestResult{0.0, pRight, 0.0, oddsRatio(table), 0.0});
    return std::min(pRight, 1.0);
}

/** @brief 批量检验 @param tables 列联表列表 @return 检验结果列表 */
QVector<FisherExactTest::TestResult> FisherExactTest::batchTest(
    const QVector<ContingencyTable>& tables)
{
    QVector<TestResult> results;
    results.reserve(tables.size());

    for (const auto& t : tables) {
        results.append(test(t));
    }

    emit batchCompleted(tables.size());
    return results;
}

/** @brief 重置统计 */
void FisherExactTest::resetStatistics()
{
    m_stats = Stats{};
    m_timeSumMs = 0.0;
}
