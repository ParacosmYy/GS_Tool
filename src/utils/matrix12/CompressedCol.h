/**
 * @file CompressedCol.h
 * @brief 压缩稀疏列(CSC)矩阵 — SpMV与Gauss-Seidel求解器
 *
 * 功能: 实现CSC(Compressed Sparse Column)稀疏矩阵存储格式，
 *       支持高效的稀疏矩阵-向量乘法(SpMV)、Gauss-Seidel迭代
 *       求解线性方程组、矩阵转置以及元素级运算。
 *
 * 协作: BandMatrix(带状矩阵) / ArnoldiSolver(Arnoldi迭代)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>
#include <QString>
#include <QElapsedTimer>

/**
 * @brief CSC格式稀疏矩阵
 */
class CompressedCol : public QObject {
    Q_OBJECT

public:
    /** @brief 运行统计 */
    struct Stats {
        int    totalSpMV = 0;             ///< 累计SpMV次数
        int    totalSolves = 0;           ///< 累计求解次数
        int    totalNonZeros = 0;         ///< 累计非零元素数
        double avgProcessingTimeMs = 0.0; ///< 平均处理耗时(ms)
    };

    explicit CompressedCol(QObject* parent = nullptr);

    /**
     * @brief 从COO(坐标)格式构建CSC矩阵
     * @param rows 行索引
     * @param cols 列索引
     * @param vals 非零值
     * @param numRows 行数
     * @param numCols 列数
     */
    void buildFromCOO(const QVector<int>& rows,
                      const QVector<int>& cols,
                      const QVector<double>& vals,
                      int numRows, int numCols);

    /**
     * @brief 从稠密矩阵构建CSC
     * @param dense 稠密矩阵(行优先)
     * @param dropTolerance 丢弃阈值(绝对值小于此值的视为0)
     */
    void buildFromDense(const QVector<QVector<double>>& dense,
                        double dropTolerance = 1e-12);

    /**
     * @brief 稀疏矩阵-向量乘法 y = A * x
     * @param x 输入向量(长度 = 列数)
     * @return 输出向量(长度 = 行数)
     */
    QVector<double> spMV(const QVector<double>& x) const;

    /**
     * @brief Gauss-Seidel迭代求解 Ax = b
     * @param b 右端向量
     * @param maxIter 最大迭代次数
     * @param tolerance 收敛容差
     * @return (解向量x, 实际迭代次数, 最终残差)
     */
    QPair<QVector<double>, QPair<int, double>>
    gaussSeidel(const QVector<double>& b,
                int maxIter = 1000,
                double tolerance = 1e-10);

    /**
     * @brief 矩阵转置
     * @return 转置后的CSC矩阵(新对象)
     */
    CompressedCol* transpose() const;

    /**
     * @brief 提取对角线元素
     * @return 对角线值数组
     */
    QVector<double> diagonal() const;

    /** @brief 获取行数 */
    int rows() const { return m_rows; }
    /** @brief 获取列数 */
    int cols() const { return m_cols; }
    /** @brief 获取非零元素数 */
    int nonZeros() const { return m_values.size(); }
    /** @brief 获取稀疏度(非零占比) */
    double sparsity() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief SpMV完成 @param nnz 非零元素数 */
    void spMVCompleted(int nnz);
    /** @brief 求解完成 @param iterations 迭代次数 @param residual 最终残差 */
    void solveCompleted(int iterations, double residual);

private:
    /** @brief 对列内元素按行索引排序 */
    void sortColumnEntries(QVector<int>& rowIdx, QVector<double>& vals,
                           int start, int end) const;

    int m_rows = 0;             ///< 行数
    int m_cols = 0;             ///< 列数

    QVector<double> m_values;   ///< 非零值(CSC顺序)
    QVector<int>    m_rowIdx;   ///< 行索引
    QVector<int>    m_colPtr;   ///< 列指针(m_cols+1个元素)

    mutable Stats              m_stats;
    mutable double             m_timeSum = 0.0;
    mutable QElapsedTimer m_timer;
};
