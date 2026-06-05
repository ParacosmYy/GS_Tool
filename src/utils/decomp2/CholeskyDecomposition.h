/**
 * @file CholeskyDecomposition.h
 * @brief Cholesky分解 — 对称正定矩阵分解
 *
 * 功能: 将对称正定矩阵分解为L*L^T，用于线性方程组高效求解、
 *       矩阵求逆和正定性检验。复杂度约为LU分解的一半。
 *
 * 协作: LuDecomposition / ExtendedKalman(协方差矩阵)
 */
#ifndef CHOLESKYDECOMPOSITION_H
#define CHOLESKYDECOMPOSITION_H

#include <QObject>
#include <QVector>

/**
 * @brief Cholesky分解 — 对称正定矩阵
 */
class CholeskyDecomposition : public QObject {
    Q_OBJECT

public:
    /** @brief 统计信息 */
    struct Stats {
        quint64 totalDecompositions = 0;  ///< 累计分解次数
        quint64 totalSolves = 0;          ///< 累计求解次数
        double avgProcessingTimeMs = 0.0; ///< 平均处理时间(ms)
    };

    /** @brief 分解结果 */
    struct CholeskyResult {
        QVector<QVector<double>> L;       ///< 下三角矩阵
        double logDeterminant = 0.0;      ///< ln(det(A))
        bool isPositiveDefinite = false;  ///< 是否正定
    };

    explicit CholeskyDecomposition(QObject* parent = nullptr);

    /** @brief 执行Cholesky分解
     *  @param matrix 对称正定矩阵
     *  @return 分解结果 */
    CholeskyResult decompose(const QVector<QVector<double>>& matrix);

    /** @brief 使用分解结果求解Ax=b
     *  @param result 分解结果
     *  @param b 右端向量
     *  @return 解向量 */
    QVector<double> solve(const CholeskyResult& result,
                          const QVector<double>& b);

    /** @brief 正定性检验
     *  @param matrix 输入矩阵
     *  @return 是否对称正定 */
    bool isPositiveDefinite(const QVector<QVector<double>>& matrix);

    /** @brief 获取统计 */
    const Stats& stats() const { return m_stats; }

    /** @brief 重置统计 */
    void resetStatistics();

signals:
    /** @brief 分解完成 @param n 维度 @param positiveDefinite 是否正定 */
    void decompositionCompleted(int n, bool positiveDefinite);

private:
    double m_timeSum;   ///< 处理时间累加器
    Stats m_stats;      ///< 统计信息
};

#endif // CHOLESKYDECOMPOSITION_H
