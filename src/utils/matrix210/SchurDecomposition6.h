/**
 * @file SchurDecomposition6.h
 * @brief Schur分解(多移位QR迭代+激进早期收缩) — Schur Decomposition with Multishift QR Iteration and Aggressive Early Deflation
 *
 * 功能: 实现实Schur分解，支持多移位QR迭代、
 *       激进早期收缩策略和实/复特征值提取。
 *
 * 协作: EigenDecomposition8(特征分解) / Householder5(Householder变换) / Hessenberg4(Hessenberg归约)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief Schur分解(多移位QR迭代+激进早期收缩)
 */
class SchurDecomposition6 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int matrixSize = 0;
        int iterations = 0;
        int deflations = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit SchurDecomposition6(QObject *parent = nullptr);
    ~SchurDecomposition6() override;

    void setMaxIterations(int maxIter);
    void setTolerance(double tol);

    /** @brief Compute real Schur decomposition: A = Q * T * Q^T */
    bool decompose(const QVector<QVector<double>>& matrix);

    /** @brief Get Schur form T (upper quasi-triangular) */
    QVector<QVector<double>> schurForm() const;

    /** @brief Get orthogonal matrix Q */
    QVector<QVector<double>> orthogonalMatrix() const;

    /** @brief Extract eigenvalues from Schur form */
    QVector<QPair<double, double>> eigenvalues() const;

    /** @brief Reduce to upper Hessenberg form via Householder */
    static void hessenbergReduce(QVector<QVector<double>>& A,
                                  QVector<QVector<double>>& Q);

    /** @brief Perform one Francis double-shift QR step */
    void francisQRStep(QVector<QVector<double>>& H, int lo, int hi,
                        QVector<QVector<double>>& Q);

    /** @brief Aggressive early deflation check */
    bool tryDeflation(QVector<QVector<double>>& H, int& lo, int& hi);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void decompositionCompleted(int size, int iterations, double timeMs);

private:
    int m_maxIter = 300;
    double m_tol = 1e-12;

    QVector<QVector<double>> m_T;  // Schur form
    QVector<QVector<double>> m_Q;  // Orthogonal matrix

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief 2x2 real Schur form of a submatrix */
    void schur2x2(QVector<QVector<double>>& H, int p, int q,
                   QVector<QVector<double>>& Q);

    /** @brief Givens rotation: zero out element */
    static void givensRotation(double a, double b, double& c, double& s);
};
