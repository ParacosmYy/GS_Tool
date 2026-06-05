#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief SymmetricEigen14 - 对称矩阵特征值分解第14代实现
 *
 * 针对实对称矩阵的高效特征值/特征向量计算，
 * 支持三对角化、QR迭代、分而治之及指定特征值范围提取。
 */
class SymmetricEigen14 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalDecompositions = 0; double avgProcessingTimeMs = 0.0; };
    explicit SymmetricEigen14(QObject* parent = nullptr);
    Stats stats() const { return m_stats; }
    void resetStatistics();

    /**
     * @brief 计算全部特征值和特征向量
     * @param matrix 对称矩阵
     * @return 特征值-特征向量对，按特征值降序排列
     */
    QVector<QPair<double, QVector<double>>> compute(const QVector<QVector<double>>& matrix);

    /**
     * @brief 仅计算特征值（不计算特征向量，速度更快）
     * @param matrix 对称矩阵
     * @return 特征值列表，降序排列
     */
    QVector<double> eigenvaluesOnly(const QVector<QVector<double>>& matrix);

    /**
     * @brief 计算指定范围内的特征值
     * @param matrix 对称矩阵
     * @param minVal 特征值下界
     * @param maxVal 特征值上界
     * @return 范围内的特征值-特征向量对
     */
    QVector<QPair<double, QVector<double>>> eigenvaluesInRange(
        const QVector<QVector<double>>& matrix, double minVal, double maxVal);

    /**
     * @brief 设置求解算法
     * @param algorithm 算法名称 (QR/DivideConquer/Jacobi/Bisection)
     */
    void setAlgorithm(const QString& algorithm);

signals:
    void decompositionCompleted(int eigenCount);

private:
    Stats m_stats; double m_timeSum = 0.0;
};
