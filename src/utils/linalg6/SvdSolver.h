/**
 * @file SvdSolver.h
 * @brief SVD奇异值分解求解器
 */

#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 奇异值分解(SVD)求解器
 *
 * 使用单边Jacobi方法进行SVD分解,
 * 支持矩阵伪逆和低秩近似。
 */
class SvdSolver : public QObject
{
    Q_OBJECT

public:
    /** @brief 统计信息 */
    struct Stats {
        int totalDecompositions = 0;    ///< 总分解次数
        int totalPseudoInverses = 0;    ///< 总伪逆次数
        double avgProcessingTimeMs = 0.0;
    };

    explicit SvdSolver(QObject* parent = nullptr);

    /**
     * @brief 执行SVD分解: A = U * S * V^T
     * @param A 输入矩阵(m x n)
     * @param U 左奇异矩阵(输出)
     * @param S 奇异值(输出,降序)
     * @param V 右奇异矩阵(输出)
     * @return 是否成功
     */
    bool decompose(const QVector<QVector<double>>& A,
                   QVector<QVector<double>>& U,
                   QVector<double>& S,
                   QVector<QVector<double>>& V);

    /**
     * @brief 计算伪逆
     * @param A 输入矩阵
     * @param tolerance 奇异值截断阈值
     * @return 伪逆矩阵
     */
    QVector<QVector<double>> pseudoInverse(
        const QVector<QVector<double>>& A,
        double tolerance = 1e-10);

    /**
     * @brief 低秩近似
     * @param A 输入矩阵
     * @param rank 目标秩
     * @return 低秩近似矩阵
     */
    QVector<QVector<double>> lowRankApproximation(
        const QVector<QVector<double>>& A, int rank);

    /**
     * @brief 计算矩阵条件数
     */
    double conditionNumber(const QVector<QVector<double>>& A);

    /**
     * @brief 计算矩阵2-范数(最大奇异值)
     */
    double matrixNorm(const QVector<QVector<double>>& A);

    Stats stats() const;
    void resetStatistics();

signals:
    /** @brief 分解完成信号 */
    void decomposed(int m, int n, int rank);

private:
    Stats m_stats;
    double m_timeSum = 0.0;

    double columnDot(const QVector<QVector<double>>& M, int c1, int c2) const;
    void rotateColumns(QVector<QVector<double>>& M, int c1, int c2,
                        double cos, double sin);
};
