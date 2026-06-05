#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief SylvesterSolver3 - Sylvester方程求解器
 *
 * 求解矩阵方程AX + XB = C，使用Bartels-Stewart
 * 算法将问题转化为Schur分解后的三角系统。
 */
class SylvesterSolver3 : public QObject {
    Q_OBJECT
public:
    struct Stats {
        int totalEquationsSolved = 0;
        int totalSize = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit SylvesterSolver3(QObject* parent = nullptr);

    /** @brief 求解AX + XB = C */
    QVector<QVector<double>> solve(
        const QVector<QVector<double>>& A,
        const QVector<QVector<double>>& B,
        const QVector<QVector<double>>& C);

    /** @brief 求解Lyapunov方程AX + XA^T = C(对称) */
    QVector<QVector<double>> solveLyapunov(
        const QVector<QVector<double>>& A,
        const QVector<QVector<double>>& C);

    /** @brief 检查方程解的残差 */
    double residual(const QVector<QVector<double>>& X,
                    const QVector<QVector<double>>& A,
                    const QVector<QVector<double>>& B,
                    const QVector<QVector<double>>& C) const;

    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    void equationSolved(int size, double residualNorm);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
};
