/**
 * @file SparseMatrixCSR.h
 * @brief 压缩稀疏行(CSR)矩阵 — 高效稀疏线性代数运算
 *
 * 功能:
 *   - CSR格式存储稀疏矩阵，内存占用O(nnz)
 *   - 矩阵向量乘法(SpMV): y = A*x
 *   - 矩阵转置(原地CSR格式)
 *   - 元素插入/查询/修改
 *   - 支持统计操作次数、非零元素数、处理时间
 */

#pragma once

#include <QObject>
#include <QVector>

/**
 * @class SparseMatrixCSR
 * @brief 压缩稀疏行(CSR)格式矩阵
 *
 * 使用三个一维数组存储稀疏矩阵:
 *   - values[]: 非零元素值
 *   - colIndices[]: 非零元素的列索引
 *   - rowPtr[]: 每行起始位置指针
 *
 * 适用于科学计算、有限元分析、图算法等稀疏矩阵场景。
 */
class SparseMatrixCSR : public QObject
{
    Q_OBJECT

public:
    /** @brief 统计信息 */
    struct Stats {
        int totalSpMV = 0;            /**< 总SpMV运算次数 */
        int totalTransposes = 0;      /**< 总转置次数 */
        int totalElementsInserted = 0; /**< 总插入元素数 */
        double avgProcessingTimeMs = 0.0; /**< 平均处理时间(ms) */
    };

    /** @brief 三元组: (行, 列, 值) */
    struct Triplet {
        int row;
        int col;
        double value;
    };

    /**
     * @brief 构造函数
     * @param rows 行数
     * @param cols 列数
     * @param parent 父对象
     */
    explicit SparseMatrixCSR(int rows = 0, int cols = 0,
                              QObject* parent = nullptr);

    /**
     * @brief 从三元组列表构建CSR矩阵
     * @param triplets 三元组列表
     */
    void buildFromTriplets(const QVector<Triplet>& triplets);

    /**
     * @brief 设置矩阵维度
     * @param rows 行数
     * @param cols 列数
     */
    void resize(int rows, int cols);

    /**
     * @brief 获取元素值
     * @param row 行索引
     * @param col 列索引
     * @return 元素值(若不存在返回0)
     */
    double get(int row, int col) const;

    /**
     * @brief 稀疏矩阵向量乘法(SpMV): y = A * x
     * @param x 输入向量(长度必须等于列数)
     * @return 结果向量
     */
    QVector<double> multiplyVector(const QVector<double>& x) const;

    /**
     * @brief 矩阵转置，返回新的CSR矩阵
     * @return 转置矩阵
     */
    SparseMatrixCSR transpose() const;

    /**
     * @brief 获取指定行的非零元素
     * @param row 行索引
     * @return (列索引, 值)列表
     */
    QVector<QPair<int, double>> getRow(int row) const;

    /**
     * @brief 清空矩阵
     */
    void clear();

    /** @brief 行数 */
    int rows() const { return m_rows; }

    /** @brief 列数 */
    int cols() const { return m_cols; }

    /** @brief 非零元素数 */
    int nonZeroCount() const { return m_values.size(); }

    /** @brief 稀疏度(非零元素占比) */
    double sparsity() const;

    /** @brief 获取统计信息 */
    Stats stats() const;

    /** @brief 重置统计信息 */
    void resetStatistics();

signals:
    /** @brief SpMV完成信号 */
    void spMVCompleted(int nonZeros, double elapsedMs);

    /** @brief 转置完成信号 */
    void transposeCompleted(int rows, int cols);

private:
    int m_rows;                  /**< 行数 */
    int m_cols;                  /**< 列数 */
    QVector<double> m_values;    /**< 非零元素值 */
    QVector<int> m_colIndices;   /**< 列索引 */
    QVector<int> m_rowPtr;       /**< 行指针 */
    mutable Stats m_stats;       /**< 统计信息 */
    mutable double m_timeSum = 0.0; /**< 累计时间 */
};
