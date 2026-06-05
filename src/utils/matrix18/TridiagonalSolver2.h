/**
 * @file TridiagonalSolver2.h
 * @brief 一般三对角方程组求解器 — Thomas算法/循环约化/多RHS
 *
 * 功能: 实现一般三对角线性方程组Ax=d的求解，支持Thomas算法、
 *       循环约化(Cyclic Reduction)并行算法、多右端向量同时求解。
 *       适用于样条插值、有限差分PDE、隐式时间步进等场景。
 *
 * 协作: DataInterpolator(插值) / BandMatrix(带状矩阵)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 三对角方程组求解器
 *
 * 三对角矩阵: 只有主对角线a[i]、下对角线b[i]、上对角线c[i]非零。
 * Thomas算法: O(n)时间, O(n)空间。
 * 循环约化: O(n)时间, 可并行化。
 */
class TridiagonalSolver2 : public QObject {
    Q_OBJECT

public:
    /** @brief 求解结果 */
    struct SolveResult {
        QVector<double> solution;     ///< 解向量x
        double determinant = 0.0;     ///< 行列式(可选计算)
        bool success = false;        ///< 是否求解成功
        QString method;              ///< 使用的求解方法名
    };

    /** @brief 多右端求解结果 */
    struct MultiRhsResult {
        QVector<QVector<double>> solutions;  ///< 多组解向量
        bool success = false;
        QString method;
    };

    /** @brief 操作统计 */
    struct Stats {
        quint64 totalSolves = 0;          ///< 总求解次数
        quint64 totalMultiRhsSolves = 0;  ///< 总多右端求解次数
        quint64 totalDimensions = 0;      ///< 累计矩阵维度
        quint64 totalFailures = 0;        ///< 总失败次数
        double  avgProcessingTimeMs = 0.0; ///< 平均处理耗时(ms)
    };

    /** @brief 构造函数 @param parent 父对象 */
    explicit TridiagonalSolver2(QObject* parent = nullptr);

    /** @brief 析构函数 */
    ~TridiagonalSolver2() override;

    // ── Thomas算法 ──

    /**
     * @brief Thomas算法求解(标准串行)
     * @param lower 下对角线 b[1..n-1] (n-1个元素)
     * @param main 主对角线 a[0..n-1] (n个元素)
     * @param upper 上对角线 c[0..n-2] (n-1个元素)
     * @param rhs 右端向量 d[0..n-1]
     * @return 求解结果
     */
    SolveResult solveThomas(const QVector<double>& lower,
                            const QVector<double>& main,
                            const QVector<double>& upper,
                            const QVector<double>& rhs);

    // ── 循环约化 ──

    /**
     * @brief 循环约化算法求解(可并行)
     * @param lower 下对角线
     * @param mainDiag 主对角线
     * @param upper 上对角线
     * @param rhs 右端向量
     * @return 求解结果
     */
    SolveResult solveCyclicReduction(const QVector<double>& lower,
                                     const QVector<double>& mainDiag,
                                     const QVector<double>& upper,
                                     const QVector<double>& rhs);

    // ── 多右端 ──

    /**
     * @brief 多右端向量同时求解(Thomas算法, 共享分解)
     * @param lower 下对角线
     * @param mainDiag 主对角线
     * @param upper 上对角线
     * @param rhsList 多组右端向量
     * @return 多组解向量
     */
    MultiRhsResult solveMultiRhs(const QVector<double>& lower,
                                 const QVector<double>& mainDiag,
                                 const QVector<double>& upper,
                                 const QVector<QVector<double>>& rhsList);

    // ── 辅助 ──

    /**
     * @brief 计算三对角矩阵行列式
     * @param lower 下对角线
     * @param mainDiag 主对角线
     * @param upper 上对角线
     * @return 行列式值
     */
    double determinant(const QVector<double>& lower,
                       const QVector<double>& mainDiag,
                       const QVector<double>& upper) const;

    // ── 统计 ──

    /** @brief 获取统计 */
    Stats stats() const;

    /** @brief 重置统计 */
    void resetStatistics();

signals:
    /** @brief 求解完成 @param dimension 维度 @param method 方法名 */
    void solveCompleted(int dimension, const QString& method);

private:
    Stats m_stats;                    ///< 操作统计
    double m_timeSum = 0.0;          ///< 累计耗时
};
