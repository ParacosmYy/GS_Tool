/**
 * @file SparseLU.h
 * @brief 稀疏LU分解求解器 — 带部分主元消元
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QMap>
#include <QPair>

/**
 * @brief 稀疏LU分解求解器
 * COO格式存储+部分主元选取+RHS前代回代
 */
class SparseLU : public QObject
{
    Q_OBJECT

public:
    /** @brief 统计信息 */
    struct Stats {
        int totalDecompositions = 0;      ///< 累计分解次数
        int totalSolves = 0;              ///< 累计求解次数
        int totalNonZeros = 0;            ///< 累计非零元数
        double avgProcessingTimeMs = 0.0;
    };

    explicit SparseLU(QObject* parent = nullptr);

    /** @brief 从COO三元组构建矩阵 @param rows 行索引 @param cols 列索引 @param values 值 @param n 矩阵阶数 */
    void buildFromCOO(const QVector<int>& rows, const QVector<int>& cols,
                      const QVector<double>& values, int n);

    /** @brief 执行LU分解(部分主元) @return 是否成功(奇异矩阵返回false) */
    bool decompose();

    /** @brief 求解Ax=b @param b 右端向量 @return 解向量 */
    QVector<double> solve(const QVector<double>& b) const;

    /** @brief 行列式(分解后可用) */
    double determinant() const { return m_det; }

    /** @brief 条件数估计(1-范数) */
    double conditionEstimate() const;

    /** @brief 获取非零元计数 */
    int nonZeroCount() const;

    /** @brief 获取矩阵阶数 */
    int order() const { return m_n; }

    /** @brief 获取主元置换 */
    QVector<int> permutation() const { return m_pivot; }

    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 分解完成 @param nnz 非零元数 @param det 行列式 */
    void decompositionCompleted(int nnz, double det);

private:
    /** @brief 稀疏行(列索引→值) */
    using SparseRow = QMap<int, double>;

    int m_n = 0;                         ///< 矩阵阶数
    QVector<SparseRow> m_L;              ///< 下三角(行稀疏)
    QVector<SparseRow> m_U;              ///< 上三角(行稀疏)
    QVector<int> m_pivot;                ///< 主元置换
    double m_det = 1.0;                  ///< 行列式
    bool m_decomposed = false;           ///< 是否已分解

    Stats m_stats;
    double m_timeSum = 0.0;
};
