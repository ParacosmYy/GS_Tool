/**
 * @file HouseholderQR.h
 * @brief QR分解 — Householder反射
 *
 * 功能: 将矩阵分解为正交Q和上三角R的乘积。用于线性最小二乘、
 *       特征值计算(QR迭代)和矩阵求解。
 *
 * 协作: LuDecomposition / SingularValueDecomposition
 */
#ifndef HOUSEHOLDERQR_H
#define HOUSEHOLDERQR_H

#include <QObject>
#include <QVector>

/**
 * @brief QR分解 — Householder方法
 */
class HouseholderQR : public QObject {
    Q_OBJECT

public:
    /** @brief 统计信息 */
    struct Stats {
        quint64 totalDecompositions = 0;  ///< 累计分解次数
        double avgProcessingTimeMs = 0.0; ///< 平均处理时间(ms)
    };

    /** @brief 分解结果 */
    struct QrResult {
        QVector<QVector<double>> Q;       ///< 正交矩阵
        QVector<QVector<double>> R;       ///< 上三角矩阵
        bool success = false;             ///< 是否成功
    };

    explicit HouseholderQR(QObject* parent = nullptr);

    /** @brief 执行QR分解
     *  @param matrix 输入矩阵(m x n, m >= n)
     *  @return 分解结果 */
    QrResult decompose(const QVector<QVector<double>>& matrix);

    /** @brief 求解最小二乘问题 min ||Ax-b||
     *  @param A 系数矩阵
     *  @param b 右端向量
     *  @return 最小二乘解 */
    QVector<double> solveLeastSquares(
        const QVector<QVector<double>>& A,
        const QVector<double>& b);

    /** @brief 获取统计 */
    const Stats& stats() const { return m_stats; }

    /** @brief 重置统计 */
    void resetStatistics();

signals:
    /** @brief 分解完成 @param rows 行数 @param cols 列数 */
    void decompositionCompleted(int rows, int cols);

private:
    double m_timeSum;   ///< 处理时间累加器
    Stats m_stats;      ///< 统计信息
};

#endif // HOUSEHOLDERQR_H
