#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief SparseLU6 - 稀疏LU分解第6代实现
 *
 * 针对大规模稀疏矩阵的高效LU分解，支持部分主元选取、
 * 填充元排序优化及条件数估计。
 */
class SparseLU6 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalDecompositions = 0; double avgProcessingTimeMs = 0.0; };
    explicit SparseLU6(QObject* parent = nullptr);
    Stats stats() const { return m_stats; }
    void resetStatistics();

    /**
     * @brief 执行稀疏LU分解 PA = LU
     * @param matrix 稀疏矩阵（行压缩存储）
     * @return 是否分解成功
     */
    bool decompose(const QVector<QVector<QPair<int, double>>>& matrix);

    /**
     * @brief 利用已有分解求解线性方程组 Ax = b
     * @param rhs 右端向量 b
     * @return 解向量 x
     */
    QVector<double> solve(const QVector<double>& rhs);

    /**
     * @brief 估计矩阵条件数
     * @return 条件数估计值
     */
    double estimateConditionNumber() const;

    /**
     * @brief 设置填充元排序策略
     * @param strategy 排序策略名称 (AMD/RCM/Natural)
     */
    void setFillReducingOrder(const QString& strategy);

signals:
    void decompositionCompleted(int matrixSize);

private:
    Stats m_stats; double m_timeSum = 0.0;
};
