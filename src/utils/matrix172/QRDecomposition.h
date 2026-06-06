/**
 * @file QRDecomposition.h
 * @brief QR分解(Householder反射+列主元+秩估计) — QR Decomposition via Householder Reflectors with Column Pivoting and Rank Estimation
 *
 * 功能: 实现QR分解，采用Householder反射变换，支持列主元选取，
 *       提供秩估计、线性最小二乘解、正交矩阵Q和上三角矩阵R提取。
 *
 * 协作: SVDecomposition(SVD) / LUDeomposition(LU) / EigenSolver(特征值)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief QR分解器(Householder列主元)
 */
class QRDecomposition : public QObject {
    Q_OBJECT

public:
    /** @brief 分解结果 */
    struct QRResult {
        QVector<QVector<double>> Q;  ///< 正交矩阵(m×m)
        QVector<QVector<double>> R;  ///< 上三角矩阵(m×n)
        QVector<int> permutation;    ///< 列置换向量
        int rank = 0;                ///< 矩阵的数值秩
    };

    /** @brief 运行统计 */
    struct Stats {
        quint64 totalDecompositions = 0; ///< 累计分解次数
        double avgProcessingTimeMs = 0.0;///< 平均耗时(ms)
        int lastRows = 0;                ///< 最近行数
        int lastCols = 0;                ///< 最近列数
        int lastRank = 0;                ///< 最近秩
    };

    explicit QRDecomposition(QObject *parent = nullptr);
    ~QRDecomposition() override;

    /** @brief 设置秩估计阈值 */
    void setTolerance(double tol);

    /**
     * @brief 执行带列主元的QR分解
     * @param A 输入矩阵(m×n, m>=n)
     * @return QR分解结果
     */
    QRResult decompose(const QVector<QVector<double>>& A);

    /**
     * @brief 最小二乘求解 Ax≈b
     * @param A 系数矩阵
     * @param b 右端向量
     * @return 最小二乘解x
     */
    QVector<double> solve(const QVector<QVector<double>>& A,
                          const QVector<double>& b);

    /** @brief 计算矩阵的秩 */
    static int computeRank(const QVector<QVector<double>>& A, double tol = 1e-10);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 分解完成 @param rows 行数 @param cols 列数 @param rank 秩 */
    void decompositionCompleted(int rows, int cols, int rank);

private:
    /** @brief Householder向量 */
    static QVector<double> householderVector(const QVector<double>& x);

    /** @brief 应用Householder变换到矩阵 */
    static void applyHouseholder(QVector<QVector<double>>& M, int col,
                                 const QVector<double>& v, int startRow);

    /** @brief 矩阵向量乘 */
    static QVector<double> matVec(const QVector<QVector<double>>& M,
                                  const QVector<double>& v);

    /** @brief 转置矩阵 */
    static QVector<QVector<double>> transpose(
        const QVector<QVector<double>>& M);

    double m_tolerance = 1e-10;

    Stats m_stats;
    double m_timeSum = 0.0;
};
