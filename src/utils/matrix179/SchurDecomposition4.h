/**
 * @file SchurDecomposition4.h
 * @brief Schur分解(Francis双移QR+复特征值提取) — Schur Decomposition via Francis Double-Shift QR with Complex Eigenvalue Extraction
 *
 * 功能: 实现矩阵Schur分解，支持Francis双移QR迭代、
 *       上Hessenberg约化和复特征值提取。
 *
 * 协作: EigenDecomposition(特征值分解) / SvdDecomposition(SVD) / QrDecomposition(QR分解)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief Schur分解器(Francis双移QR)
 */
class SchurDecomposition4 : public QObject {
    Q_OBJECT

public:
    /** @brief 分解结果 */
    struct Result {
        QVector<QVector<double>> T;  ///< 上三角Schur矩阵T
        QVector<QVector<double>> Q;  ///< 正交变换矩阵Q (A = Q*T*Q^T)
        QVector<QPair<double, double>> eigenvalues; ///< 特征值(real, imag)
        int iterations = 0;          ///< QR迭代次数
        bool converged = false;      ///< 是否收敛
    };

    /** @brief 运行统计 */
    struct Stats {
        quint64 totalDecompositions = 0;  ///< 累计分解次数
        int matrixSize = 0;               ///< 最近矩阵阶数
        int lastIterations = 0;           ///< 最近迭代次数
        double avgProcessingTimeMs = 0.0; ///< 平均耗时(ms)
    };

    explicit SchurDecomposition4(QObject *parent = nullptr);
    ~SchurDecomposition4() override;

    void setMaxIterations(int iter);
    void setConvergenceThreshold(double eps);

    /**
     * @brief 执行实Schur分解
     * @param matrix 实对称或一般方阵
     * @return 分解结果(T, Q, eigenvalues)
     */
    Result decompose(const QVector<QVector<double>>& matrix);

    /** @brief 仅提取特征值 */
    QVector<QPair<double, double>> eigenvalues(
        const QVector<QVector<double>>& matrix);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void decompositionCompleted(int size, int iterations, bool converged);

private:
    /** @brief 约化到上Hessenberg形式 */
    void hessenbergReduce(QVector<QVector<double>>& A,
                          QVector<QVector<double>>& Q) const;

    /** @brief Francis双移QR步 */
    void francisDoubleShift(QVector<QVector<double>>& A,
                            QVector<QVector<double>>& Q,
                            int p, int q);

    /** @brief Givens旋转 */
    void applyGivens(QVector<QVector<double>>& A,
                     QVector<QVector<double>>& Q,
                     int i, int j, double c, double s,
                     bool left) const;

    /** @brief 提取2x2块的特征值 */
    static QPair<double, double> eigenvalues2x2(
        const QVector<QVector<double>>& A, int i);

    /** @brief 检查子矩阵是否收敛 */
    bool checkConvergence(const QVector<QVector<double>>& A) const;

    /** @brief 初始化单位矩阵 */
    static QVector<QVector<double>> identity(int n);

    int m_maxIter = 1000;
    double m_eps = 1e-12;

    Stats m_stats;
    double m_timeSum = 0.0;
};
