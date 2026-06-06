/**
 * @file SymmetricEigenSolver.h
 * @brief 对称特征值求解(三对角化+隐式QR Wilkinson移位) — Symmetric Eigenvalue Problem via Tridiagonalization and Implicit QR with Wilkinson Shift
 *
 * 功能: 实现对称矩阵特征值求解，支持Householder三对角化、
 *       隐式QR迭代(Wilkinson移位)和特征向量计算。
 *
 * 协作: MatrixDecomp(矩阵分解) / SvdSolver(SVD) / ConditionNumber(条件数)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 对称特征值求解器(三对角化+隐式QR)
 */
class SymmetricEigenSolver : public QObject {
    Q_OBJECT

public:
    /** @brief 运行统计 */
    struct Stats {
        quint64 totalSolves = 0;          ///< 累计求解次数
        int matrixSize = 0;               ///< 矩阵尺寸
        int qrIterations = 0;             ///< QR迭代次数
        double residual = 0.0;            ///< 残差范数
        double avgProcessingTimeMs = 0.0; ///< 平均耗时(ms)
    };

    explicit SymmetricEigenSolver(QObject *parent = nullptr);
    ~SymmetricEigenSolver() override;

    /** @brief 设置收敛阈值 */
    void setTolerance(double tol);

    /** @brief 设置最大迭代次数 */
    void setMaxIterations(int iter);

    /**
     * @brief 求解对称矩阵的全部特征值
     * @param matrix 对称矩阵(n×n, 行优先)
     * @return 特征值(升序排列)
     */
    QVector<double> solve(const QVector<QVector<double>>& matrix);

    /**
     * @brief 求解特征值和特征向量
     * @param matrix 对称矩阵
     * @return {eigenvalues, eigenvectors} 每列为一个特征向量
     */
    QPair<QVector<double>, QVector<QVector<double>>>
    solveWithVectors(const QVector<QVector<double>>& matrix);

    /** @brief 只计算前k个特征值 */
    QVector<double> solvePartial(
        const QVector<QVector<double>>& matrix, int k);

    void reset();
    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void solveCompleted(int n, int iterations);
    void iterationProgress(int current, int total);

private:
    /** @brief Householder三对角化 */
    void tridiagonalize(QVector<QVector<double>>& mat,
                        QVector<QVector<double>>& Q);

    /** @brief 隐式QR步(Wilkinson移位) */
    void implicitQRStep(QVector<double>& diag, QVector<double>& sub,
                        QVector<QVector<double>>& Q, int lo, int hi);

    /** @brief 计算Wilkinson移位 */
    static double wilkinsonShift(double d1, double d2, double e);

    /** @brief 计算残差 */
    static double computeResidual(
        const QVector<QVector<double>>& mat,
        const QVector<double>& eigenvalues,
        const QVector<QVector<double>>& eigenvectors);

    double m_tolerance = 1e-12;
    int m_maxIterations = 100;

    Stats m_stats;
    double m_timeSum = 0.0;
};
