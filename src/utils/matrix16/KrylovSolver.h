/**
 * @file KrylovSolver.h
 * @brief Krylov子空间方法 — GMRES迭代求解大规模稀疏线性系统
 *
 * 功能: 实现广义最小残差法(GMRES)，支持稀疏矩阵存储，
 *       Arnoldi正交化、Givens旋转求解上Hessenberg最小二乘。
 *       适用于非对称稀疏系统，内置收敛监测和预处理对角缩放。
 *
 * 协作: DataTransformer(矩阵序列化) / TrendPredictor(预测迭代)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief Krylov子空间求解器 — GMRES求解大规模稀疏线性系统Ax=b
 */
class KrylovSolver : public QObject {
    Q_OBJECT

public:
    /** @brief 稀疏矩阵COO格式三元组 */
    struct SparseEntry {
        int row = 0;            ///< 行索引
        int col = 0;            ///< 列索引
        double value = 0.0;     ///< 非零值
    };

    /** @brief 求解结果 */
    struct SolveResult {
        QVector<double> x;              ///< 解向量
        double residualNorm = 0.0;      ///< 最终残差范数
        int iterationsUsed = 0;         ///< 实际迭代次数
        bool converged = false;         ///< 是否收敛
    };

    /** @brief 预处理类型 */
    enum class Preconditioner {
        None,               ///< 无预处理
        Diagonal,           ///< 对角(Jacobi)缩放
        SymmetricGaussSeidel///< 对称Gauss-Seidel
    };
    Q_ENUM(Preconditioner)

    /** @brief 运行时统计信息 */
    struct Stats {
        int totalSolves = 0;                    ///< 累计求解次数
        int totalIterations = 0;                ///< 累计迭代次数
        int totalConverged = 0;                 ///< 累计收敛次数
        double avgProcessingTimeMs = 0.0;       ///< 平均处理耗时(ms)
    };

    /** @brief 构造函数 @param parent 父对象 */
    explicit KrylovSolver(QObject* parent = nullptr);

    /** @brief 设置矩阵维度 @param n 维度 */
    void setDimension(int n);

    /** @brief 设置稀疏矩阵(COO格式) @param entries 三元组列表 */
    void setSparseMatrix(const QVector<SparseEntry>& entries);

    /** @brief 设置最大迭代次数 @param maxIter 最大迭代 */
    void setMaxIterations(int maxIter);

    /** @brief 设置收敛容差 @param tol 相对残差容差 */
    void setTolerance(double tol);

    /** @brief 设置预处理类型 @param pc 预处理类型 */
    void setPreconditioner(Preconditioner pc);

    /** @brief 求解Ax=b @param b 右端向量 @return 求解结果 */
    SolveResult solve(const QVector<double>& b);

    /** @brief 矩阵-向量乘积y=A*x @param x 输入向量 @return 乘积向量 */
    QVector<double> multiply(const QVector<double>& x) const;

    /** @brief 获取当前统计 @return 统计常量引用 */
    const Stats& stats() const { return m_stats; }

    /** @brief 重置统计计数器 */
    void resetStatistics();

signals:
    /** @brief 迭代进度 @param iter 当前迭代 @param residual 当前残差 */
    void iterationProgress(int iter, double residual);

private:
    /** @brief Arnoldi正交化一步 @param V 基向量 @param h Hessenberg列 @param k 当前步 */
    void arnoldiStep(QVector<QVector<double>>& V,
                     QVector<double>& h, int k);

    /** @brief Givens旋转求解最小二乘 @param H Hessenberg矩阵 @param g 右端变换 @param k 当前步数 */
    QVector<double> solveHessenberg(
        const QVector<QVector<double>>& H,
        const QVector<double>& g, int k);

    /** @brief 对角预处理: 前推和回代 @param x 输入/输出向量 */
    void applyPreconditioner(QVector<double>& x) const;

    int m_dimension;                            ///< 矩阵维度
    int m_maxIter;                              ///< 最大迭代次数
    double m_tolerance;                         ///< 收敛容差
    Preconditioner m_precond;                   ///< 预处理类型

    /** @brief CSR存储 */
    QVector<int> m_rowPtr;                      ///< 行指针
    QVector<int> m_colIdx;                      ///< 列索引
    QVector<double> m_values;                   ///< 非零值
    QVector<double> m_diagInv;                  ///< 对角逆(Jacobi)

    Stats m_stats;                              ///< 运行时统计
    double m_timeSum = 0.0;                     ///< 累计耗时(ms)
};
