#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief SchurDecomp7 - Schur分解第7代实现
 *
 * 提供实Schur分解和复Schur分解，将矩阵分解为
 * Q * T * Q^H 形式，支持排序Schur向量及特征值提取。
 */
class SchurDecomp7 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalDecompositions = 0; double avgProcessingTimeMs = 0.0; };
    explicit SchurDecomp7(QObject* parent = nullptr);
    Stats stats() const { return m_stats; }
    void resetStatistics();

    /**
     * @brief 执行实Schur分解 A = Q T Q^T
     * @param matrix 输入实矩阵
     * @return 是否分解成功
     */
    bool decomposeReal(const QVector<QVector<double>>& matrix);

    /**
     * @brief 从Schur分解结果提取特征值
     * @return 特征值列表（可能含复共轭对）
     */
    QVector<QPair<double, double>> extractEigenvalues() const;

    /**
     * @brief 按模排序Schur分解（用于特征值子空间提取）
     * @param sortAscending 是否按模升序排列
     * @return 排序后的正交矩阵Q
     */
    QVector<QVector<double>> orderSchur(bool sortAscending = false);

    /**
     * @brief 获取上三角Schur矩阵T
     * @return Schur矩阵
     */
    QVector<QVector<double>> schurMatrix() const;

signals:
    void decompositionCompleted(int matrixSize);

private:
    Stats m_stats; double m_timeSum = 0.0;
};
