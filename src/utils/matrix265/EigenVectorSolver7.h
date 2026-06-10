/**
 * @file EigenVectorSolver7.h
 * @brief 特征向量求解器(Davidson方法预处理校正方程内部特征值计算) — Eigenvector Solver with Davidson Method and Preconditioned Correction Equation for Interior Eigenvalue Computation
 *
 * 功能: 实现特征向量求解器(eigenvector solver)，采用Davidson方法(Davidson
 *       method)和预处理校正方程(preconditioned correction equation)进行
 *       内部特征值计算(interior eigenvalue computation)。
 *
 * 协作: EigenValue8(特征值) / SVD9(奇异值分解) / MatrixInverse7(矩阵求逆)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 特征向量求解器(Davidson方法预处理校正方程内部特征值计算)
 */
class EigenVectorSolver7 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int matrixSize = 0;
        int numEigenvaluesFound = 0;
        int totalIterations = 0;
        double lastResidual = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    /** @brief Eigenvalue-eigenvector pair result */
    struct EigenPair {
        double eigenvalue = 0.0;
        QVector<double> eigenvector;
        double residual = 0.0;
    };

    explicit EigenVectorSolver7(QObject *parent = nullptr);
    ~EigenVectorSolver7() override;

    /** @brief Set target number of eigenvalues to compute */
    void setNumEigenvalues(int num);

    /** @brief Set convergence tolerance for residual norm */
    void setTolerance(double tol);

    /** @brief Set maximum iterations per eigenvalue */
    void setMaxIterations(int iters);

    /** @brief Set maximum subspace dimension before restart */
    void setMaxSubspaceSize(int size);

    /** @brief Solve for eigenvalues (smallest magnitude by default) */
    QVector<EigenPair> solve(const QVector<QVector<double>>& matrix);

    /** @brief Solve for eigenvalues closest to a target shift */
    QVector<EigenPair> solveShifted(const QVector<QVector<double>>& matrix,
                                     double shift);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void eigenvalueFound(int index, double value, double residual, double timeMs);
    void iterationCompleted(int iter, double residual, int subspaceSize);

private:
    int m_numEigenvalues = 1;
    double m_tolerance = 1e-8;
    int m_maxIter = 200;
    int m_maxSubspace = 30;
    int m_n = 0;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Matrix-vector multiplication */
    QVector<double> matVec(const QVector<QVector<double>>& mat,
                            const QVector<double>& vec) const;

    /** @brief Preconditioned correction: t = (D - sigma*I)^{-1} * r */
    QVector<double> precondition(const QVector<QVector<double>>& mat,
                                  const QVector<double>& residual,
                                  double shift) const;

    /** @brief Orthonormalize vector against existing basis using modified Gram-Schmidt */
    void orthonormalize(QVector<double>& vec,
                        const QVector<QVector<double>>& basis) const;

    /** @brief Solve small dense eigenvalue problem via QR iteration */
    EigenPair solveSmallDense(const QVector<QVector<double>>& smallMat) const;

    /** @brief Compute residual: r = A*v - lambda*v */
    QVector<double> computeResidual(const QVector<QVector<double>>& mat,
                                     const EigenPair& pair) const;

    /** @brief Vector dot product */
    double dot(const QVector<double>& a, const QVector<double>& b) const;

    /** @brief Vector L2 norm */
    double norm(const QVector<double>& v) const;
};
