#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief Schur分解求解器实现 (版本6)
 *
 * 将一般方阵分解为 Q*T*Q^H 形式，T为上三角矩阵(实Schur形式为拟上三角)，
 * Q为正交/酉矩阵，是计算特征值和矩阵函数的重要工具。
 */
class SchurDecomp6 : public QObject {
    Q_OBJECT
public:
    /// 统计信息结构体
    struct Stats { int totalDecomposed = 0; double avgProcessingTimeMs = 0.0; };

    explicit SchurDecomp6(QObject* parent = nullptr);

    /** @brief 设置待分解矩阵的维度 */
    void setDimension(int n);

    /** @brief 设置矩阵元素(行,列,值) */
    void setEntry(int row, int col, double value);

    /** @brief 执行实Schur分解，返回(T, Q)矩阵对 */
    QPair<QVector<QVector<double>>, QVector<QVector<double>>> decompose();

    /** @brief 从Schur形式提取特征值 */
    QVector<QPair<double, double>> eigenvalues() const { return m_eigenvalues; }

    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 分解完成信号，返回矩阵维度 */
    void decompositionCompleted(int dimension);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
    int m_dimension = 0;
    QVector<QPair<double, double>> m_eigenvalues;
};
