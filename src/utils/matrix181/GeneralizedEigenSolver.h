/**
 * @file GeneralizedEigenSolver.h
 * @brief 广义特征值问题(QZ分解+QZ步进+降阶) — Generalized Eigenvalue Problem QZ Decomposition via QZ Step with Deflation
 *
 * 功能: 实现广义特征值问题Ax=λBx的QZ分解求解，支持QZ步进迭代、
 *       降阶策略、广义Schur分解和特征向量计算。
 *
 * 协作: EigenDecomp4(特征分解) / SvdSolver4(SVD) / Hessenberg4(Hessenberg归约)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 广义特征值求解器(QZ分解)
 */
class GeneralizedEigenSolver : public QObject {
    Q_OBJECT

public:
    /** @brief 特征值(复数: 实部, 虚部) */
    struct EigenValue {
        double alphaRe = 0.0;   ///< 分子实部
        double alphaIm = 0.0;   ///< 分子虚部
        double beta = 0.0;      ///< 分母(实数)
        bool isFinite() const { return qAbs(beta) > 1e-15; }
        double realPart() const { return isFinite() ? alphaRe / beta : 0.0; }
        double imagPart() const { return isFinite() ? alphaIm / beta : 0.0; }
    };

    /** @brief 运行统计 */
    struct Stats {
        quint64 totalSolves = 0;
        int matrixSize = 0;
        int qzIterations = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit GeneralizedEigenSolver(QObject *parent = nullptr);
    ~GeneralizedEigenSolver() override;

    /**
     * @brief 求解广义特征值问题 Ax = λBx
     * @param A 矩阵A (n×n)
     * @param B 矩阵B (n×n)
     * @return 广义特征值列表
     */
    QVector<EigenValue> solve(const QVector<QVector<double>>& A,
                               const QVector<QVector<double>>& B);

    /** @brief 获取广义Schur形式(S,T) */
    QPair<QVector<QVector<double>>, QVector<QVector<double>>>
    schurForm() const;

    /** @brief 获取QZ分解的正交矩阵Q,Z */
    QPair<QVector<QVector<double>>, QVector<QVector<double>>>
    qzMatrices() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void solveCompleted(int n, int iterations);

private:
    QVector<QVector<double>> m_S;  ///< Schur form of A
    QVector<QVector<double>> m_T;  ///< Schur form of B
    QVector<QVector<double>> m_Q;  ///< Left orthogonal
    QVector<QVector<double>> m_Z;  ///< Right orthogonal

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Hessenberg reduction of A, upper triangular of B */
    void hessenbergTriangular(QVector<QVector<double>>& A,
                               QVector<QVector<double>>& B);

    /** @brief One QZ step (implicit double shift) */
    void qzStep(QVector<QVector<double>>& A, QVector<QVector<double>>& B,
                int lo, int hi);

    /** @brief Givens rotation application */
    void applyGivens(QVector<QVector<double>>& M, int i, int j,
                     double c, double s, bool fromLeft);
};
