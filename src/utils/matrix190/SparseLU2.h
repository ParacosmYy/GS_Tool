/**
 * @file SparseLU2.h
 * @brief 稀疏LU分解(列/行置换填入减少+阈值部分主元) — Sparse LU Factorization with Column/Row Permutation for Fill-Reduction and Threshold Partial Pivoting
 *
 * 功能: 实现稀疏LU分解，支持填入减少排序(AMD近似最小度)、
 *       阈值部分主元选取、三角求解和条件数估计。
 *
 * 协作: SparseMatrix4(稀疏矩阵) / Cholesky5(Cholesky) / QRDecomp3(QR分解)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 稀疏LU分解器(填入减少+阈值主元)
 */
class SparseLU2 : public QObject {
    Q_OBJECT

public:
    /** @brief 运行统计 */
    struct Stats {
        quint64 totalFactorizations = 0;
        int matrixSize = 0;
        int fillInCount = 0;
        double pivotingThreshold = 0.1;
        double conditionEstimate = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    /** @brief 分解结果 */
    struct LUFactorization {
        QVector<QVector<QPair<int, double>>> L; // sparse lower triangular
        QVector<QVector<QPair<int, double>>> U; // sparse upper triangular
        QVector<int> rowPerm;                   // row permutation P
        QVector<int> colPerm;                   // column permutation Q
    };

    explicit SparseLU2(QObject *parent = nullptr);
    ~SparseLU2() override;

    void setThreshold(double threshold);

    /** @brief 执行稀疏LU分解 */
    LUFactorization factorize(int n,
                              const QVector<QVector<QPair<int, double>>>& cols);

    /** @brief 三角求解 Ax=b */
    QVector<double> solve(const LUFactorization& lu,
                          const QVector<double>& b) const;

    /** @brief 近似最小度排序(AMD) */
    QVector<int> amdOrdering(int n,
                             const QVector<QVector<QPair<int, double>>>& cols) const;

    /** @brief 计算填入量 */
    int computeFillIn(const LUFactorization& lu) const;

    /** @brief 条件数估计(1-norm) */
    double conditionEstimate(const LUFactorization& lu,
                             int n) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void factorizationCompleted(int n, int fillIn, double timeMs);

private:
    double m_threshold = 0.1;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Threshold partial pivoting */
    int selectPivot(const QVector<QVector<QPair<int, double>>>& U,
                    int col, int startRow,
                    const QVector<bool>& usedRows) const;

    /** @brief Sparse scatter-add */
    void scatterAdd(QVector<double>& denseCol,
                    const QVector<QPair<int, double>>& sparseRow,
                    double factor) const;
};
