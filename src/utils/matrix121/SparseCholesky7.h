#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief SparseCholesky7 - 稀疏Cholesky分解第7代实现
 *
 * 针对对称正定稀疏矩阵的Cholesky分解 A = LL^T，
 * 支持填充缩减排序、非零模式预测及多右端项求解。
 */
class SparseCholesky7 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalDecompositions = 0; double avgProcessingTimeMs = 0.0; };
    explicit SparseCholesky7(QObject* parent = nullptr);
    Stats stats() const { return m_stats; }
    void resetStatistics();

    /**
     * @brief 执行稀疏Cholesky分解 A = LL^T
     * @param matrix 对称正定稀疏矩阵
     * @return 是否分解成功
     */
    bool decompose(const QVector<QVector<QPair<int, double>>>& matrix);

    /**
     * @brief 利用分解结果求解 Ax = b
     * @param rhs 右端向量 b
     * @return 解向量 x
     */
    QVector<double> solve(const QVector<double>& rhs);

    /**
     * @brief 预测填充模式（非零元素位置）
     * @param matrix 稀疏矩阵
     * @return 填充后的非零模式
     */
    QVector<QVector<int>> predictFillPattern(
        const QVector<QVector<QPair<int, double>>>& matrix) const;

    /**
     * @brief 设置填充缩减排序
     * @param ordering 排序方法 (AMD/NestedDissection/Natural)
     */
    void setOrdering(const QString& ordering);

signals:
    void decompositionCompleted(int matrixSize);

private:
    Stats m_stats; double m_timeSum = 0.0;
};
