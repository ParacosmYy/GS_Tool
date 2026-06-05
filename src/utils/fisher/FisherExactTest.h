/**
 * @file FisherExactTest.h
 * @brief Fisher精确检验 — 2×2列联表的精确概率计算
 *
 * 功能: 对2×2列联表执行Fisher精确检验，计算p-value，
 *       支持双侧和单侧检验，适用于小样本场景。
 *
 * 协作: StatDistribution(分布检验) / DataClassifier(分类评估)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

#include <cmath>

/**
 * @brief Fisher精确检验 — 2×2列联表统计检验
 */
class FisherExactTest : public QObject {
    Q_OBJECT

public:
    /** @brief 2×2列联表数据 */
    struct ContingencyTable {
        qint64 a = 0;   ///< 左上格 (A+, B+)
        qint64 b = 0;   ///< 右上格 (A+, B-)
        qint64 c = 0;   ///< 左下格 (A-, B+)
        qint64 d = 0;   ///< 右下格 (A-, B-)
    };

    /** @brief 检验结果 */
    struct TestResult {
        double pValueLeft     = 0.0;   ///< 左侧p值
        double pValueRight    = 0.0;   ///< 右侧p值
        double pValueTwoSided = 0.0;   ///< 双侧p值
        double oddsRatio      = 0.0;   ///< 优势比
        double logOddsRatio   = 0.0;   ///< 对数优势比
    };

    /** @brief 统计信息 */
    struct Stats {
        quint64 totalTests          = 0;   ///< 累计检验次数
        quint64 totalTablesComputed = 0;   ///< 累计列联表计算数
        double  avgProcessingTimeMs = 0.0; ///< 平均处理时间(ms)
        double  minPValue           = 1.0; ///< 最小p值
        double  maxPValue           = 0.0; ///< 最大p值
    };

    explicit FisherExactTest(QObject* parent = nullptr);

    /**
     * @brief 执行Fisher精确检验(双侧)
     * @param table 2×2列联表
     * @return 检验结果(p值、优势比)
     */
    TestResult test(const ContingencyTable& table);

    /**
     * @brief 执行Fisher精确检验(单侧，右侧)
     * @param table 2×2列联表
     * @return 右侧p值
     */
    double testOneSided(const ContingencyTable& table);

    /**
     * @brief 计算优势比
     * @param table 2×2列联表
     * @return 优势比 (a*d)/(b*c)，含边界保护
     */
    double oddsRatio(const ContingencyTable& table) const;

    /**
     * @brief 批量检验
     * @param tables 列联表列表
     * @return 检验结果列表
     */
    QVector<TestResult> batchTest(const QVector<ContingencyTable>& tables);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 检验完成 @param result 检验结果 */
    void testCompleted(const TestResult& result);

    /** @brief 批量检验完成 @param count 批次大小 */
    void batchCompleted(int count);

private:
    /**
     * @brief 计算给定列联表的概率
     * @param table 列联表
     * @return 超几何分布概率
     */
    double tableProbability(const ContingencyTable& table) const;

    /**
     * @brief 对数阶乘(避免溢出)
     * @param n 非负整数
     * @return ln(n!)
     */
    double logFactorial(qint64 n) const;

    Stats  m_stats;
    double m_timeSumMs = 0.0;  ///< 累计耗时
};
