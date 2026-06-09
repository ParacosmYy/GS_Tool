/**
 * @file SchurDecomposition9.h
 * @brief Schur分解(AED激进提前收缩+多位移QR聚类特征值分离) — Schur Decomposition with AED Aggressive Early Deflation and Multishift QR for Clustered Eigenvalue Separation
 *
 * 功能: 实现Schur分解(Schur Decomposition)，使用AED激进提前收缩
 *       (Aggressive Early Deflation)加速收敛，通过多位移QR(multishift QR)
 *       处理聚类特征值分离。
 *
 * 协作: EigenDecomposition9(特征值分解) / SvdDecomposition9(SVD分解) / QRDecomposition9(QR分解)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief Schur分解(AED提前收缩+多位移QR)
 */
class SchurDecomposition9 : public QObject {
    Q_OBJECT

public:
    /** @brief Complex number as real pair */
    using Complex = QPair<double, double>;

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int matrixSize = 0;
        int numIterations = 0;
        int numDeflations = 0;
        int numShifts = 0;
        bool converged = false;
        double avgProcessingTimeMs = 0.0;
    };

    explicit SchurDecomposition9(QObject *parent = nullptr);
    ~SchurDecomposition9() override;

    /** @brief Set maximum QR iterations */
    void setMaxIterations(int maxIter);

    /** @brief Set convergence tolerance */
    void setTolerance(double tol);

    /** @brief Compute Schur decomposition: A = Q * T * Q^H */
    bool decompose(const QVector<QVector<double>>& matrix);

    /** @brief Get unitary Q matrix */
    QVector<QVector<double>> matrixQ() const;

    /** @brief Get upper quasi-triangular T matrix */
    QVector<QVector<double>> matrixT() const;

    /** @brief Extract eigenvalues from T */
    QVector<Complex> eigenvalues() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void decompositionCompleted(int size, int iterations, bool converged, double timeMs);

private:
    int m_maxIterations = 300;
    double m_tolerance = 1e-12;

    QVector<QVector<double>> m_Q;
    QVector<QVector<double>> m_T;
    int m_n = 0;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Hessenberg reduction: A -> H */
    void hessenbergReduce(QVector<QVector<double>>& A,
                           QVector<QVector<double>>& Q);

    /** @brief Apply Householder reflection */
    void applyHouseholder(QVector<QVector<double>>& A, int row, int col,
                           QVector<QVector<double>>& Q);

    /** @brief Multishift QR step with k shifts */
    void multishiftQRStep(QVector<QVector<double>>& H,
                           QVector<QVector<double>>& Q,
                           int lo, int hi);

    /** @brief AED aggressive early deflation */
    int aggressiveDeflation(QVector<QVector<double>>& H,
                             QVector<QVector<double>>& Q,
                             int lo, int hi);

    /** @brief Check for deflation (small subdiagonal) */
    int checkDeflation(const QVector<QVector<double>>& H,
                        int lo, int hi) const;

    /** @brief Francis double-shift QR step */
    void francisDoubleShift(QVector<QVector<double>>& H,
                             QVector<QVector<double>>& Q,
                             int lo, int hi);

    /** @brief Extract 2x2 eigenvalue pair */
    QPair<Complex, Complex> eigenvalue2x2(
        const QVector<QVector<double>>& H, int i) const;
};
