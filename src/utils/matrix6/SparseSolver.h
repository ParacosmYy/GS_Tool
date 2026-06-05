/**
 * @file SparseSolver.h
 * @brief 稀疏线性求解器 — 预条件共轭梯度法(CG)求解大型稀疏对称正定方程组
 *
 * 功能: 使用 Jacobi/SSOR 预条件的共轭梯度迭代法求解 Ax=b，
 *       支持自定义收敛阈值、最大迭代次数、残差监控，
 *       适用于有限元/差分网格等稀疏矩阵场景。
 *
 * 协作: DataInterpolator(数据插值) / SpectrumAnalyzer(频谱分析)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QMap>
#include <QPair>

/**
 * @brief 稀疏线性求解器 — 预条件共轭梯度法
 *
 * 典型用法:
 * @code
 *   SparseSolver solver;
 *   solver.setTolerance(1e-8);
 *   solver.setMaxIterations(500);
 *   solver.buildMatrix(100, triples);
 *   QVector<double> x = solver.solve(b);
 * @endcode
 */
class SparseSolver : public QObject {
    Q_OBJECT

public:
    /** @brief 预条件类型 */
    enum class Preconditioner {
        None,       ///< 无预条件
        Jacobi,     ///< Jacobi对角预条件
        SSOR        ///< 对称超松弛预条件
    };
    Q_ENUM(Preconditioner)

    /** @brief 迭代结果 */
    struct SolveResult {
        QVector<double> solution;       ///< 解向量
        int iterations = 0;             ///< 实际迭代次数
        double residualNorm = 0.0;      ///< 最终残差范数
        bool converged = false;         ///< 是否收敛
        double elapsedMs = 0.0;         ///< 求解耗时(ms)
    };

    /** @brief 统计数据 */
    struct Stats {
        int totalSolves = 0;                ///< 累计求解次数
        int totalIterations = 0;            ///< 累计迭代次数
        double avgProcessingTimeMs = 0.0;   ///< 平均处理时间(ms)
        int totalConverged = 0;             ///< 收敛次数
        int totalDiverged = 0;              ///< 未收敛次数
    };

    explicit SparseSolver(QObject* parent = nullptr);

    /** @brief 设置收敛阈值 @param tol 残差阈值 */
    void setTolerance(double tol);

    /** @brief 设置最大迭代次数 @param maxIter 最大迭代 */
    void setMaxIterations(int maxIter);

    /** @brief 设置预条件类型 @param pc 预条件器 */
    void setPreconditioner(Preconditioner pc);

    /** @brief 设置SSOR松弛因子 @param omega 松弛因子(0,2) */
    void setOmega(double omega);

    /**
     * @brief 从三元组构建稀疏矩阵(CSR格式)
     * @param n 矩阵维度
     * @param triples (row, col, value)三元组列表
     */
    void buildMatrix(int n, const QVector<QTriple<int, int, double>>& triples);

    /**
     * @brief 求解 Ax = b
     * @param b 右端向量(长度须等于矩阵维度)
     * @return 求解结果
     */
    SolveResult solve(const QVector<double>& b);

    /**
     * @brief 矩阵-向量乘法 y = Ax
     * @param x 输入向量
     * @return 结果向量
     */
    QVector<double> multiply(const QVector<double>& x) const;

    /** @brief 获取统计 @return 统计数据 */
    Stats stats() const { return m_stats; }

    /** @brief 重置统计 */
    void resetStatistics();

signals:
    /** @brief 求解完成 @param result 求解结果 */
    void solveCompleted(const SolveResult& result);

private:
    /** @brief CSR稀疏矩阵存储 */
    struct SparseMatrix {
        int n = 0;                          ///< 维度
        QVector<int> rowPtr;                ///< 行指针
        QVector<int> colIdx;                ///< 列索引
        QVector<double> values;             ///< 非零值
    };

    /** @brief 应用预条件 M^{-1}r -> z */
    QVector<double> applyPreconditioner(const QVector<double>& r) const;

    /** @brief Jacobi预条件 */
    QVector<double> jacobiPrecond(const QVector<double>& r) const;

    /** @brief SSOR前向回代 */
    QVector<double> ssorForward(const QVector<double>& r) const;

    /** @brief SSOR后向回代 */
    QVector<double> ssorBackward(const QVector<double>& r) const;

    /** @brief 计算对角线元素 */
    void extractDiagonal();

    /** @brief 内积 */
    static double dot(const QVector<double>& a, const QVector<double>& b);

    /** @brief 向量范数 */
    static double norm(const QVector<double>& v);

    SparseMatrix m_matrix;          ///< 稀疏矩阵
    QVector<double> m_diag;         ///< 对角线元素
    double m_tolerance = 1e-6;      ///< 收敛阈值
    int m_maxIter = 200;            ///< 最大迭代
    Preconditioner m_pc = Preconditioner::Jacobi;
    double m_omega = 1.0;           ///< SSOR松弛因子
    Stats m_stats;                  ///< 统计数据
    double m_timeSum = 0.0;         ///< 时间累加器
};
