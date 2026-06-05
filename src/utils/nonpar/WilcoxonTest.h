/**
 * @file WilcoxonTest.h
 * @brief Wilcoxon符号秩检验(Wilcoxon signed-rank test)
 *
 * 功能: 实现非参数统计中的Wilcoxon符号秩检验，
 *       用于比较配对样本或单样本中位数的差异。
 *       支持正态近似p值计算。统计检验次数/平均耗时。
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @class WilcoxonTest
 * @brief Wilcoxon符号秩检验 — 配对样本非参数检验
 *
 * 计算配对差值的符号秩和W统计量，并通过正态近似
 * 计算双侧p值。适用于样本量n > 10的场景。
 */
class WilcoxonTest : public QObject
{
    Q_OBJECT

public:
    /** @brief 运行统计信息 */
    struct Stats {
        quint64 totalTests = 0;       ///< 总检验次数
        double  avgProcessingTimeMs = 0.0; ///< 平均处理耗时(ms)
    };

    /**
     * @brief 构造函数
     * @param parent QObject父对象
     */
    explicit WilcoxonTest(QObject* parent = nullptr);

    /**
     * @brief 执行Wilcoxon符号秩检验(配对样本)
     * @param x 第一组样本数据
     * @param y 第二组样本数据(与x等长)
     * @return W统计量(W+)
     */
    double test(const QVector<double>& x, const QVector<double>& y);

    /**
     * @brief 执行单样本检验(与中位数比较)
     * @param x 样本数据
     * @param median 假设的中位数(默认0)
     * @return W统计量(W+)
     */
    double testOneSample(const QVector<double>& x, double median = 0.0);

    /** @brief 获取最近一次检验的p值 */
    double pValue() const { return m_pValue; }

    /** @brief 获取最近一次检验的W统计量 */
    double statistic() const { return m_statistic; }

    /** @brief 获取统计信息 */
    Stats stats() const { return m_stats; }

    /** @brief 重置统计信息 */
    void resetStatistics();

signals:
    /** @brief 检验完成信号 @param statistic W统计量 @param pValue 双侧p值 */
    void testCompleted(double statistic, double pValue);

private:
    /**
     * @brief 计算符号秩和
     * @param differences 配对差值列表
     * @return W+统计量
     */
    double computeSignedRankSum(QVector<double> differences) const;

    /**
     * @brief 通过正态近似计算p值
     * @param wPlus W+统计量
     * @param n 非零差值个数
     * @return 双侧p值
     */
    double normalApproxPValue(double wPlus, int n) const;

    mutable Stats m_stats;     ///< 统计信息
    double m_timeSum = 0.0;    ///< 累计耗时
    double m_statistic = 0.0;  ///< 最近W统计量
    double m_pValue = 0.0;     ///< 最近p值
};
