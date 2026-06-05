/**
 * @file HouseholderQR2.h
 * @brief Householder QR分解 — 正交三角分解与线性求解
 *
 * 功能: 将矩阵A分解为正交矩阵Q和上三角矩阵R(A=QR)，
 *       用于线性方程组求解、最小二乘问题和特征值计算。
 *
 * 协作: LuDecomposition / SingularValueDecomposition / EigenSolver
 */
#ifndef HOUSEHOLDERQR3_H
#define HOUSEHOLDERQR3_H

#include <QObject>
#include <QVector>

/**
 * @brief Householder QR分解器
 *
 * 使用Householder反射逐步将矩阵化为上三角形式。
 * Q存储为Householder向量，可按需显式构造。
 */
class HouseholderQR2 : public QObject {
    Q_OBJECT

public:
    /** @brief 统计信息 */
    struct Stats {
        quint64 totalComputed = 0;         ///< 累计分解次数
        double  avgProcessingTimeMs = 0.0; ///< 平均处理耗时(ms)
    };

    explicit HouseholderQR2(QObject* parent = nullptr);

    /**
     * @brief 执行QR分解
     * @param matrix 输入矩阵(m×n, m >= n)
     */
    void compute(const QVector<QVector<double>>& matrix);

    /**
     * @brief 获取正交矩阵Q
     * @return Q矩阵(m×m)
     */
    QVector<QVector<double>> Q() const;

    /**
     * @brief 获取上三角矩阵R
     * @return R矩阵(m×n)
     */
    QVector<QVector<double>> R() const;

    /**
     * @brief 求解线性方程组 Ax = b
     * @param b 右端向量
     * @return 解向量x
     */
    QVector<double> solve(const QVector<double>& b) const;

    /** @brief 是否已计算 */
    bool isComputed() const { return m_computed; }

    /** @brief 获取统计信息 */
    const Stats& stats() const { return m_stats; }

    /** @brief 重置统计信息 */
    void resetStatistics();

signals:
    /** @brief 分解完成信号 @param rows 行数 @param cols 列数 */
    void computed(int rows, int cols);

private:
    bool m_computed = false;                ///< 是否已计算
    int m_rows = 0;                         ///< 矩阵行数
    int m_cols = 0;                         ///< 矩阵列数
    QVector<QVector<double>> m_R;           ///< 上三角矩阵
    QVector<QVector<double>> m_householders;///< Householder向量
    Stats m_stats;                          ///< 统计信息
    double m_timeSumMs = 0.0;               ///< 累计耗时(ms)
};

#endif // HOUSEHOLDERQR3_H
