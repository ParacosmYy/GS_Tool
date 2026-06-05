/**
 * @file SingularValueSolver.h
 * @brief 精简SVD求解器 — Thin SVD via Golub-Kahan Bidiagonalization + QR Shifts
 *
 * 功能: 计算矩阵的精简奇异值分解 A = UΣV^T。使用Golub-Kahan
 *       双对角化预处理，再通过隐式QR位移迭代计算奇异值。
 *
 * 协作: EigenDecomposition(特征分解) / QRDecomposition(QR分解)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 精简SVD求解器
 */
class SingularValueSolver : public QObject {
    Q_OBJECT

public:
    /** @brief SVD结果 */
    struct SVDResult {
        QVector<QVector<double>> U;     ///< 左奇异向量矩阵(m×k)
        QVector<double> singularValues; ///< 奇异值(k个，降序)
        QVector<QVector<double>> V;     ///< 右奇异向量矩阵(n×k)
    };

    /** @brief 运行统计 */
    struct Stats {
        quint64 totalDecompositions = 0;    ///< 累计分解次数
        double avgProcessingTimeMs = 0.0;   ///< 平均处理耗时(ms)
        double conditionNumber = 0.0;       ///< 最近一次的条件数
        int matrixRows = 0;                 ///< 最近一次矩阵行数
        int matrixCols = 0;                 ///< 最近一次矩阵列数
    };

    explicit SingularValueSolver(QObject* parent = nullptr);

    /**
     * @brief 设置收敛阈值
     * @param eps 收敛判据
     */
    void setEpsilon(double eps);

    /**
     * @brief 设置最大迭代次数
     * @param maxIter 最大迭代次数
     */
    void setMaxIterations(int maxIter);

    /**
     * @brief 计算精简SVD
     * @param A 输入矩阵(m×n)，m >= n
     * @return SVD结果
     */
    SVDResult decompose(const QVector<QVector<double>>& A);

    /**
     * @brief 仅计算奇异值(不计算U和V)
     * @param A 输入矩阵
     * @return 奇异值(降序)
     */
    QVector<double> singularValuesOnly(const QVector<QVector<double>>& A);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief SVD完成 @param m 行数 @param n 列数 @param iterations 迭代次数 */
    void decompositionCompleted(int m, int n, int iterations);

private:
    /** @brief Householder向量 */
    QVector<double> householderVector(const QVector<double>& x, int start) const;

    /** @brief Golub-Kahan双对角化 */
    void bidiagonalize(QVector<QVector<double>>& A,
                       QVector<double>& diagonal,
                       QVector<double>& superDiagonal);

    /** @brief 隐式QR位移迭代 */
    int qrShiftIteration(QVector<double>& diagonal,
                         QVector<double>& superDiagonal,
                         QVector<QVector<double>>& U,
                         QVector<QVector<double>>& V);

    /** @brief Givens旋转 */
    void givensRotation(double a, double b, double& c, double& s) const;

    double m_eps = 1e-12;
    int m_maxIter = 200;

    Stats m_stats;
    double m_timeSum = 0.0;
};
