/**
 * @file SchurDecomposition5.h
 * @brief Schur分解(特征值聚类重排序+不变子空间提取) — Schur Decomposition with Reordering for Eigenvalue Clustering and Invariant Subspace Extraction
 *
 * 功能: 实现Schur分解，支持QR迭代计算上三角Schur形式、
 *       特征值聚类重排序和不变子空间提取。
 *
 * 协作: EigenDecomposition9(特征分解) / QrDecomposition8(QR分解) / Svd7(SVD)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief Schur分解(特征值聚类重排序+不变子空间提取)
 */
class SchurDecomposition5 : public QObject {
    Q_OBJECT

public:
    /** @brief 运行统计 */
    struct Stats {
        quint64 totalDecompositions = 0;
        int matrixSize = 0;
        int qrIterations = 0;
        int reorderSwaps = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit SchurDecomposition5(QObject *parent = nullptr);
    ~SchurDecomposition5() override;

    void setMaxIterations(int iter);
    void setTolerance(double tol);

    /** @brief Compute Schur decomposition: A = Q * T * Q^T */
    bool decompose(const QVector<QVector<double>>& A);

    /** @brief Reorder Schur form to cluster selected eigenvalues */
    bool reorder(const QVector<int>& selected);

    /** @brief Extract invariant subspace for columns [0..k-1] */
    QVector<QVector<double>> invariantSubspace(int k) const;

    /** @brief Get Schur form matrix T (upper triangular) */
    QVector<QVector<double>> schurForm() const { return m_T; }

    /** @brief Get orthogonal transformation matrix Q */
    QVector<QVector<double>> transformation() const { return m_Q; }

    /** @brief Get eigenvalues (diagonal of T, possibly complex pairs) */
    QVector<QPair<double, double>> eigenvalues() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void decompositionCompleted(int n, int iterations, double timeMs);

private:
    int m_maxIterations = 300;
    double m_tolerance = 1e-10;

    QVector<QVector<double>> m_T;
    QVector<QVector<double>> m_Q;
    int m_size = 0;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Householder reflection for column k */
    void householderQR(QVector<QVector<double>>& R,
                       QVector<QVector<double>>& QAccum) const;

    /** @brief Apply Francis double shift QR step */
    void francisStep(QVector<QVector<double>>& H,
                     QVector<QVector<double>>& QAccum, int lo, int hi);

    /** @brief Swap adjacent Schur blocks at position k */
    void swapBlocks(int k);

    /** @brief Matrix multiply C = A * B */
    QVector<QVector<double>> matMul(const QVector<QVector<double>>& A,
                                     const QVector<QVector<double>>& B) const;

    /** @brief Identity matrix of size n */
    QVector<QVector<double>> identity(int n) const;

    /** @brief Matrix transpose */
    QVector<QVector<double>> transpose(
        const QVector<QVector<double>>& A) const;
};
