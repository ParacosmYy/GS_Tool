/**
 * @file BlockMatrix.h
 * @brief 分块矩阵运算,支持分块乘法和LU分解
 */

#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 分块矩阵运算器
 *
 * 将大矩阵按固定块大小分块,实现Cache友好的矩阵乘法和
 * 分块LU分解。支持转置、矩阵-向量乘、迹计算等基础操作。
 * 适用于大规模数值计算和嵌入式信号处理。
 */
class BlockMatrix : public QObject
{
    Q_OBJECT

public:
    /** @brief 统计信息 */
    struct Stats {
        int totalMultiplications = 0;   ///< 总矩阵乘法次数
        int totalLUDecompositions = 0;  ///< 总LU分解次数
        double avgProcessingTimeMs = 0.0;///< 平均处理时间(ms)
    };

    explicit BlockMatrix(QObject* parent = nullptr);

    /**
     * @brief 分块矩阵乘法 C = A * B
     * @param A 矩阵A (m x k)
     * @param B 矩阵B (k x n)
     * @param rowsA A的行数
     * @param colsA A的列数(=B的行数)
     * @param colsB B的列数
     * @param blockSize 分块大小(默认64)
     * @return 结果矩阵C (m x n)
     */
    QVector<double> multiply(const QVector<double>& A,
                             const QVector<double>& B,
                             int rowsA, int colsA, int colsB,
                             int blockSize = 64);

    /**
     * @brief 分块LU分解(PA = LU)
     * @param matrix 输入方阵(以一维数组存储,行优先)
     * @param n 矩阵维度
     * @param blockSize 分块大小
     * @return {L矩阵, U矩阵, 置换数组P}
     */
    QVector<QVector<double>> luDecompose(const QVector<double>& matrix,
                                         int n, int blockSize = 32);

    /**
     * @brief 使用LU分解结果求解线性方程组 Ax = b
     * @param L 下三角矩阵
     * @param U 上三角矩阵
     * @param perm 置换数组
     * @param b 右端向量
     * @param n 矩阵维度
     * @return 解向量x
     */
    QVector<double> luSolve(const QVector<double>& L,
                            const QVector<double>& U,
                            const QVector<int>& perm,
                            const QVector<double>& b, int n) const;

    /**
     * @brief 矩阵转置
     * @param matrix 输入矩阵
     * @param rows 行数
     * @param cols 列数
     * @return 转置矩阵
     */
    QVector<double> transpose(const QVector<double>& matrix,
                              int rows, int cols) const;

    /**
     * @brief 矩阵-向量乘法 y = A * x
     * @param matrix 矩阵A (m x n)
     * @param vec 向量x (n)
     * @param rows 行数
     * @param cols 列数
     * @return 结果向量y (m)
     */
    QVector<double> matVecMultiply(const QVector<double>& matrix,
                                   const QVector<double>& vec,
                                   int rows, int cols) const;

    /**
     * @brief 计算矩阵的迹(对角线元素之和)
     * @param matrix 方阵
     * @param n 维度
     * @return 迹
     */
    double trace(const QVector<double>& matrix, int n) const;

    Stats stats() const;
    void resetStatistics();

signals:
    /** @brief 矩阵运算完成信号 */
    void operationCompleted(const QString& operation, int size);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
};
