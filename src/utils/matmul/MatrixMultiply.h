/**
 * @file MatrixMultiply.h
 * @brief 矩阵乘法优化(Matrix Multiplication Optimized)
 */

#pragma once

#include <QObject>
#include <QVector>

/**
 * @class MatrixMultiply
 * @brief 矩阵乘法优化 — 分块/Strassen/Winograd矩阵乘法
 *
 * 支持朴素/分块/Strassen算法自动选择、
 * 矩阵运算(C=AB, 转置, 行列式)。
 * 适用于大规模数值计算、线性代数等场景。
 */
class MatrixMultiply : public QObject
{
    Q_OBJECT

public:
    /** @brief 统计信息 */
    struct Stats {
        int totalMultiplied = 0;  /**< 总乘法次数 */
        long long totalFlops = 0; /**< 总浮点运算数 */
        double avgProcessingTimeMs = 0.0; /**< 平均处理时间(ms) */
    };

    /** @brief 构造函数 */
    explicit MatrixMultiply(QObject* parent = nullptr);

    /**
     * @brief 矩阵乘法(自动选择最优算法)
     * @param A m×k矩阵
     * @param B k×n矩阵
     * @return m×n结果矩阵
     */
    QVector<QVector<double>> multiply(
        const QVector<QVector<double>>& A,
        const QVector<QVector<double>>& B);

    /**
     * @brief 朴素矩阵乘法
     */
    QVector<QVector<double>> naive(
        const QVector<QVector<double>>& A,
        const QVector<QVector<double>>& B);

    /**
     * @brief 分块矩阵乘法(cache-friendly)
     * @param blockSize 分块大小
     */
    QVector<QVector<double>> blocked(
        const QVector<QVector<double>>& A,
        const QVector<QVector<double>>& B,
        int blockSize = 64);

    /**
     * @brief Strassen矩阵乘法(分治)
     */
    QVector<QVector<double>> strassen(
        const QVector<QVector<double>>& A,
        const QVector<QVector<double>>& B);

    /**
     * @brief 矩阵转置
     */
    static QVector<QVector<double>> transpose(
        const QVector<QVector<double>>& matrix);

    /**
     * @brief 矩阵行列式(高斯消元)
     */
    static double determinant(const QVector<QVector<double>>& matrix);

    /** @brief 获取统计 */
    Stats stats() const;

    /** @brief 重置统计 */
    void resetStatistics();

signals:
    /** @brief 乘法完成信号 */
    void multiplied(int rowsA, int colsB);

private:
    void addMat(const QVector<QVector<double>>& A,
                const QVector<QVector<double>>& B,
                QVector<QVector<double>>& C) const;
    void subMat(const QVector<QVector<double>>& A,
                const QVector<QVector<double>>& B,
                QVector<QVector<double>>& C) const;

    Stats m_stats;
    double m_timeSum;
};
