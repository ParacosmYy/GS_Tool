/**
 * @file GivensRotation.h
 * @brief Givens旋转(Givens Rotation)
 */

#pragma once

#include <QObject>
#include <QVector>

/**
 * @class GivensRotation
 * @brief Givens旋转 — 零化特定元素的旋转变换
 *
 * 支持QR分解(逐个零化)、SVD辅助、最小二乘求解。
 * 适用于稀疏矩阵处理、线性求解等场景。
 */
class GivensRotation : public QObject
{
    Q_OBJECT

public:
    /** @brief 统计信息 */
    struct Stats {
        int totalRotations = 0;   /**< 总旋转次数 */
        int totalSolves = 0;      /**< 总求解次数 */
        double avgProcessingTimeMs = 0.0; /**< 平均处理时间(ms) */
    };

    /** @brief 构造函数 */
    explicit GivensRotation(QObject* parent = nullptr);

    /**
     * @brief 计算Givens旋转参数
     * @param a 行元素
     * @param b 列元素(将被零化)
     * @return (cos, sin)
     */
    static QPair<double, double> compute(double a, double b);

    /**
     * @brief 对矩阵行应用Givens旋转
     * @param matrix 矩阵(会被修改)
     * @param i 行1
     * @param j 行2
     * @param col 应用的列
     * @param c cos值
     * @param s sin值
     */
    static void applyLeft(QVector<QVector<double>>& matrix,
                           int i, int j, int col, double c, double s);

    /**
     * @brief QR分解(使用Givens旋转)
     * @param matrix 输入矩阵(m×n)
     * @return Q矩阵(m×m)和R矩阵(m×n)
     */
    QPair<QVector<QVector<double>>, QVector<QVector<double>>> qrDecompose(
        const QVector<QVector<double>>& matrix);

    /**
     * @brief 最小二乘求解(Ax≈b)
     * @param A 系数矩阵
     * @param b 右侧向量
     * @return 最小二乘解x
     */
    QVector<double> leastSquares(const QVector<QVector<double>>& A,
                                   const QVector<double>& b);

    /** @brief 获取统计 */
    Stats stats() const;

    /** @brief 重置统计 */
    void resetStatistics();

signals:
    /** @brief 分解完成信号 */
    void decompositionCompleted(int rows, int cols);

private:
    Stats m_stats;
    double m_timeSum;
};
