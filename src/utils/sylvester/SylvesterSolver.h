/**
 * @file SylvesterSolver.h
 * @brief Sylvester方程求解器 — Bartels-Stewart算法
 *
 * 功能: 求解Sylvester矩阵方程 AX + XB = C，
 *       通过Schur分解转化为三角系统求解。
 *
 * 协作: SchurDecomposition(Schur分解) / TridiagonalSolver(三对角)
 */
#ifndef SYLVESTERSOLVER_H
#define SYLVESTERSOLVER_H

#include <QObject>
#include <QVector>

/**
 * @brief Sylvester方程求解器
 */
class SylvesterSolver : public QObject {
    Q_OBJECT

public:
    /** @brief 统计 */
    struct Stats {
        quint64 totalSolves = 0;            ///< 累计求解次数
        double  avgProcessingTimeMs = 0.0;  ///< 平均处理耗时(ms)
    };

    explicit SylvesterSolver(QObject* parent = nullptr);

    /**
     * @brief 求解Sylvester方程 AX + XB = C
     * @param A 矩阵A(m×m)
     * @param B 矩阵B(n×n)
     * @param C 矩阵C(m×n)
     * @return 解矩阵X(m×n)
     */
    QVector<QVector<double>> solve(const QVector<QVector<double>>& A,
                                    const QVector<QVector<double>>& B,
                                    const QVector<QVector<double>>& C);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 求解完成 @param size 矩阵维度 */
    void solveCompleted(int size);

private:
    Stats m_stats;              ///< 统计信息
    double m_timeSum = 0.0;     ///< 累计耗时
};

#endif // SYLVESTERSOLVER_H
