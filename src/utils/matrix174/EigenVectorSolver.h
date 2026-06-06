/**
 * @file EigenVectorSolver.h
 * @brief 特征向量求解器(反幂迭代+Rayleigh商迭代) — Eigenvector Solver via Inverse Iteration and Rayleigh Quotient Iteration
 *
 * 功能: 实现特征向量求解，支持反幂迭代法求最小特征值对应特征向量、
 *       Rayleigh商迭代加速收敛和移位策略，适用于嵌入式振动分析。
 *
 * 协作: MatrixDecomp6(LU分解) / SvdSolver8(SVD) / MatrixOps3(矩阵运算)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 特征向量求解器
 */
class EigenVectorSolver : public QObject {
    Q_OBJECT

public:
    /** @brief 运行统计 */
    struct Stats {
        quint64 totalSolves = 0;         ///< 累计求解次数
        int lastIterations = 0;          ///< 最近迭代数
        double lastEigenvalue = 0.0;     ///< 最近特征值
        double avgProcessingTimeMs = 0.0;///< 平均耗时(ms)
        bool lastConverged = false;      ///< 最近是否收敛
    };

    explicit EigenVectorSolver(QObject *parent = nullptr);
    ~EigenVectorSolver() override;

    void setMaxIterations(int maxIter);
    void setTolerance(double tol);

    /**
     * @brief 反幂迭代法求最接近shift的特征向量
     * @param matrix 对称矩阵(行优先)
     * @param shift 移位值(默认0=最小特征值)
     * @param eigenVector [out] 输出特征向量
     * @return 特征值
     */
    double inverseIteration(const QVector<QVector<double>>& matrix,
                             double shift, QVector<double>& eigenVector);

    /**
     * @brief Rayleigh商迭代(三次收敛)
     * @param matrix 对称矩阵(行优先)
     * @param initialGuess 初始向量
     * @param eigenVector [out] 输出特征向量
     * @return 特征值
     */
    double rayleighIteration(const QVector<QVector<double>>& matrix,
                              const QVector<double>& initialGuess,
                              QVector<double>& eigenVector);

    /**
     * @brief 计算Rayleigh商
     * @param matrix 矩阵
     * @param vec 向量
     * @return Rayleigh商值
     */
    static double rayleighQuotient(const QVector<QVector<double>>& matrix,
                                    const QVector<double>& vec);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 求解完成 @param eigenvalue 特征值 @param converged 是否收敛 */
    void solveCompleted(double eigenvalue, bool converged);

private:
    /** @brief 解线性方程组 Ax=b (Gaussian elimination with partial pivoting) */
    static bool solveLinear(QVector<QVector<double>> A,
                             QVector<double> b, QVector<double>& x);

    /** @brief 向量归一化 */
    static void normalize(QVector<double>& v);

    /** @brief 向量点积 */
    static double dot(const QVector<double>& a, const QVector<double>& b);

    /** @brief 矩阵向量乘 */
    static QVector<double> matVec(const QVector<QVector<double>>& M,
                                   const QVector<double>& v);

    int m_maxIter = 200;
    double m_tol = 1e-10;

    Stats m_stats;
    double m_timeSum = 0.0;
};
