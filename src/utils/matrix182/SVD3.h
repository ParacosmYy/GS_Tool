/**
 * @file SVD3.h
 * @brief SVD分解(精简/完全+Golub-Kahan双对角化+隐式零位移QR) — Thin/Full SVD via Golub-Kahan Bidiagonalization with Implicit Zero-shift QR Iteration
 *
 * 功能: 实现SVD矩阵分解，支持Golub-Kahan双对角化预处理、
 *       隐式零位移QR迭代求解奇异值和奇异向量。
 *
 * 协作: EigenSolver3(特征值) / QRDecomp3(QR分解) / LeastSquares3(最小二乘)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief SVD分解器(Golub-Kahan双对角化+隐式QR)
 */
class SVD3 : public QObject {
    Q_OBJECT

public:
    /** @brief 运行统计 */
    struct Stats {
        quint64 totalDecompositions = 0;
        int rows = 0;
        int cols = 0;
        int rank = 0;
        double conditionNumber = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit SVD3(QObject *parent = nullptr);
    ~SVD3() override;

    /** @brief 执行SVD分解: A = U * S * V^T */
    void decompose(const QVector<QVector<double>>& A);

    /** @brief 获取左奇异向量矩阵U */
    QVector<QVector<double>> matrixU() const;

    /** @brief 获取奇异值向量(降序) */
    QVector<double> singularValues() const;

    /** @brief 获取右奇异向量矩阵V */
    QVector<QVector<double>> matrixV() const;

    /** @brief 计算矩阵条件数 */
    double conditionNumber() const;

    /** @brief 计算数值秩(阈值以上的奇异值数) */
    int numericalRank(double threshold = 1e-10) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void decompositionCompleted(int rank, double cond);

private:
    int m_rows = 0, m_cols = 0;
    QVector<QVector<double>> m_U;   ///< Left singular vectors
    QVector<double> m_S;            ///< Singular values
    QVector<QVector<double>> m_V;   ///< Right singular vectors

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Golub-Kahan bidiagonalization */
    void bidiagonalize(QVector<QVector<double>>& B,
                       QVector<QVector<double>>& U,
                       QVector<QVector<double>>& V);

    /** @brief Householder reflection */
    void householder(QVector<double>& x, int start,
                     double& beta, QVector<double>& v) const;

    /** @brief Implicit zero-shift QR step on bidiagonal */
    void implicitQRStep(QVector<double>& diag, QVector<double>& superdiag,
                        QVector<QVector<double>>& U,
                        QVector<QVector<double>>& V, int lo, int hi);

    /** @brief Givens rotation application */
    void applyGivens(QVector<double>& a, QVector<double>& b,
                     double c, double s, int i, int j) const;
};
