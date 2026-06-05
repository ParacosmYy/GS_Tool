/**
 * @file SparseMatrixCRS.h
 * @brief 稀疏矩阵(CRS/CCS格式)
 */

#pragma once

#include <QObject>
#include <QVector>

/**
 * @class SparseMatrixCRS
 * @brief CRS(Compressed Row Storage)稀疏矩阵
 *
 * 高效存储和运算大规模稀疏矩阵。
 * 支持矩阵乘法、转置、向量乘等基本运算。
 */
class SparseMatrixCRS : public QObject
{
    Q_OBJECT

public:
    /** @brief 统计信息 */
    struct Stats {
        int totalOperations = 0;    /**< 总运算次数 */
        double avgProcessingTimeMs = 0.0; /**< 平均处理时间(ms) */
    };

    /**
     * @brief 构造函数
     * @param rows 行数
     * @param cols 列数
     * @param parent 父对象
     */
    explicit SparseMatrixCRS(int rows = 0, int cols = 0, QObject* parent = nullptr);

    /** @brief 设置元素值 */
    void setValue(int row, int col, double value);

    /** @brief 获取元素值 */
    double value(int row, int col) const;

    /** @brief 矩阵向量乘 */
    QVector<double> multiply(const QVector<double>& vec) const;

    /** @brief 矩阵乘法 */
    SparseMatrixCRS* multiply(const SparseMatrixCRS& other) const;

    /** @brief 转置 */
    SparseMatrixCRS* transpose() const;

    /** @brief 行数 */
    int rows() const;

    /** @brief 列数 */
    int cols() const;

    /** @brief 非零元素数 */
    int nonZeroCount() const;

    /** @brief 密度 */
    double density() const;

    /** @brief 转为密集矩阵(调试用) */
    QVector<QVector<double>> toDense() const;

    /** @brief 获取统计 */
    Stats stats() const;

    /** @brief 重置统计 */
    void resetStatistics();

signals:
    /** @brief 运算完成信号 */
    void operationCompleted(const QString& opName, double timeMs);

private:
    int m_rows, m_cols;
    QVector<double> m_values;      /**< 非零值 */
    QVector<int> m_colIdx;         /**< 列索引 */
    QVector<int> m_rowPtr;         /**< 行指针 */
    Stats m_stats;
    double m_timeSum;
};
