#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief GeneralizedEigen6 - 广义特征值问题第6代实现
 *
 * 求解广义特征值问题 Ax = λBx，支持Cholesky-Bai预处理、
 * QZ分解及束矩阵对称正定情况下的高效求解。
 */
class GeneralizedEigen6 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalDecompositions = 0; double avgProcessingTimeMs = 0.0; };
    explicit GeneralizedEigen6(QObject* parent = nullptr);
    Stats stats() const { return m_stats; }
    void resetStatistics();

    /**
     * @brief 求解广义特征值问题 Ax = λBx
     * @param matrixA 矩阵A
     * @param matrixB 矩阵B
     * @return 特征值-特征向量对
     */
    QVector<QPair<QPair<double, double>, QVector<double>>> solve(
        const QVector<QVector<double>>& matrixA,
        const QVector<QVector<double>>& matrixB);

    /**
     * @brief QZ分解（广义Schur分解）
     * @param matrixA 矩阵A
     * @param matrixB 矩阵B
     * @return 分解是否成功
     */
    bool qzDecompose(const QVector<QVector<double>>& matrixA,
                     const QVector<QVector<double>>& matrixB);

    /**
     * @brief 检查矩阵是否对称正定
     * @param matrix 待检查矩阵
     * @return 是否对称正定
     */
    bool isSymmetricPositiveDefinite(const QVector<QVector<double>>& matrix) const;

    /**
     * @brief 设置求解方法
     * @param method 方法名称 (CholeskyShift/QZ/Davidson)
     */
    void setMethod(const QString& method);

signals:
    void decompositionCompleted(int eigenCount);

private:
    Stats m_stats; double m_timeSum = 0.0;
};
