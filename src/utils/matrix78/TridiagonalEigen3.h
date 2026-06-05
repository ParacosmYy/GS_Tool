#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief TridiagonalEigen3 - 三对角矩阵特征值求解器
 *
 * 使用隐式QR算法和分治法求解三对角矩阵的
 * 全部特征值和特征向量，适用于对称三对角矩阵。
 */
class TridiagonalEigen3 : public QObject {
    Q_OBJECT
public:
    struct Stats {
        int totalEigensolves = 0;
        int totalEigenvalues = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit TridiagonalEigen3(QObject* parent = nullptr);

    /** @brief 从对角线和次对角线求解特征值 */
    QVector<double> eigenvalues(const QVector<double>& diag, const QVector<double>& subdiag);

    /** @brief 求解特征值和特征向量 */
    QPair<QVector<double>, QVector<QVector<double>>> eigenDecomposition(
        const QVector<double>& diag, const QVector<double>& subdiag);

    /** @brief 设置收敛容差 */
    void setTolerance(double tol);

    /** @brief 设置最大迭代次数 */
    void setMaxIterations(int maxIter);

    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    void eigensolveCompleted(int eigenvalueCount);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
    double m_tolerance = 1e-12;
    int m_maxIterations = 1000;
};
