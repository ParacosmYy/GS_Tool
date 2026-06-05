/**
 * @file QRDecomp2.h
 * @brief 列主元QR分解 — Householder反射/秩揭示/最小二乘
 *
 * 功能: 实现带列主元的QR分解，支持Householder反射变换、
 *       秩揭示QR、列主元选择策略、最小二乘求解。
 *
 * 协作: SpectrumAnalyzer(信号处理) / DataInterpolator(插值拟合)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QList>
#include <QPair>

/**
 * @brief 列主元QR分解 — 矩阵分解与线性求解
 */
class QRDecomp2 : public QObject {
    Q_OBJECT

public:
    /** @brief 分解结果 */
    struct QRResult {
        QVector<QVector<double>> Q;     ///< 正交矩阵Q
        QVector<QVector<double>> R;     ///< 上三角矩阵R
        QVector<int> permutation;       ///< 列置换向量
        int rank = 0;                   ///< 矩阵秩
        double conditionNumber = 0.0;   ///< 条件数估计
    };

    /** @brief 统计 */
    struct Stats {
        quint64 totalDecompositions = 0;    ///< 累计分解次数
        quint64 totalSolves = 0;            ///< 累计求解次数
        quint64 totalElementsProcessed = 0; ///< 累计处理矩阵元素数
        double  avgProcessingTimeMs = 0.0;  ///< 平均处理时间(ms)
        double  worstConditionNumber = 0.0; ///< 历史最差条件数
    };

    explicit QRDecomp2(QObject* parent = nullptr);

    /**
     * @brief 带列主元的QR分解
     * @param matrix 输入矩阵 [m x n]
     * @param tolerance 秩判定容差
     * @return QR分解结果
     */
    QRResult decompose(const QVector<QVector<double>>& matrix,
                       double tolerance = 1e-10);

    /**
     * @brief Householder反射变换
     * @param x 输入向量
     * @return (反射后向量, 反射系数beta)
     */
    QPair<QVector<double>, double> householderReflection(
        const QVector<double>& x);

    /**
     * @brief 列主元选择
     * @param matrix 当前矩阵
     * @param col 起始列
     * @param pivotCandidates 候选列索引
     * @return 最大范数列索引
     */
    int selectPivotColumn(const QVector<QVector<double>>& matrix,
                          int col, const QVector<int>& pivotCandidates);

    /**
     * @brief 最小二乘求解
     * @param A 系数矩阵
     * @param b 右侧向量
     * @return 最小二乘解
     */
    QVector<double> leastSquaresSolve(
        const QVector<QVector<double>>& A,
        const QVector<double>& b);

    /**
     * @brief 估计矩阵条件数
     * @param R 上三角矩阵
     * @return 条件数估计
     */
    double estimateConditionNumber(
        const QVector<QVector<double>>& R) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 分解完成 @param rows 行数 @param cols 列数 @param rank 秩 */
    void decompositionComplete(int rows, int cols, int rank);

    /** @brief 求解完成 @param residual 残差范数 */
    void solveComplete(double residual);

private:
    double vectorNorm(const QVector<double>& v) const;
    QVector<double> matVecMultiply(const QVector<QVector<double>>& M,
                                   const QVector<double>& v) const;
    void applyHouseholder(QVector<QVector<double>>& matrix,
                          const QVector<double>& v, double beta,
                          int startRow, int startCol);

    Stats m_stats;
    double m_timeSum = 0.0;     ///< 处理时间累加器
};
