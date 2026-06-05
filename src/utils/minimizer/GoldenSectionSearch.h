/**
 * @file GoldenSectionSearch.h
 * @brief 黄金分割搜索 — 一维函数最小化
 *
 * 功能: 利用黄金分割比(φ ≈ 0.618)在单峰区间 [a,b] 上
 *       以最少函数求值次数找到局部最小值。
 *
 * 协作: BacktrackingLineSearch(线搜索) / BisectionSolver(求根)
 */
#ifndef GOLDENSECTIONSEARCH_H
#define GOLDENSECTIONSEARCH_H

#include <QObject>
#include <functional>

/**
 * @brief 黄金分割搜索最小化器
 */
class GoldenSectionSearch : public QObject {
    Q_OBJECT

public:
    /** @brief 一元目标函数类型 */
    using Func = std::function<double(double)>;

    /** @brief 运行统计 */
    struct Stats {
        quint64 totalMinimizations = 0;  ///< 累计最小化次数
        double  avgProcessingTimeMs = 0.0; ///< 平均处理耗时(ms)
    };

    explicit GoldenSectionSearch(QObject* parent = nullptr);

    /**
     * @brief 在区间 [a,b] 上寻找 f 的最小值
     * @param f    目标函数(单峰假设)
     * @param a    区间左端点
     * @param b    区间右端点
     * @param tol  收敛容差(区间宽度)
     * @param maxIter 最大迭代次数
     * @return 近似最小值点 x*
     */
    double minimize(Func f, double a, double b,
                    double tol = 1e-10, int maxIter = 200);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 最小化完成 @param minimum 最小值点 @param iterations 迭代次数 */
    void minimizationCompleted(double minimum, int iterations);

private:
    Stats  m_stats;          ///< 统计信息
    double m_timeSum = 0.0;  ///< 累计耗时
};

#endif // GOLDENSECTIONSEARCH_H
