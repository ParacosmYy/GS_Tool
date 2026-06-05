/**
 * @file TridiagEigenSolver.h
 * @brief 三对角对称矩阵特征值求解器 — 隐式QR迭代
 *
 * 功能: 求解三对角对称矩阵的全部特征值，基于隐式QR位移迭代，
 *       适用于Lanczos/三对角化后的特征值计算，统计求解次数与平均耗时。
 *
 * 协作: SymmetricEigenSolver(稠密对称) / MinresSolver(Lanczos)
 */
#ifndef TRIDIAGEIGENSOLVER_H
#define TRIDIAGEIGENSOLVER_H

#include <QObject>
#include <QVector>

class TridiagEigenSolver : public QObject {
    Q_OBJECT

public:
    /** @brief 统计信息 */
    struct Stats {
        quint64 totalSolves    = 0;
        double  avgProcessingTimeMs = 0.0;
    };

    explicit TridiagEigenSolver(QObject* parent = nullptr);

    /**
     * @brief 求解三对角对称矩阵的特征值
     * @param diag    主对角线元素(长度n)
     * @param offDiag 次对角线元素(长度n-1)
     * @param maxIter 最大QR迭代次数(默认300)
     * @return 特征值数组(长度n，升序排列)
     */
    QVector<double> solve(const QVector<double>& diag,
                          const QVector<double>& offDiag,
                          int maxIter = 300);

    /** @brief 获取统计信息 */
    const Stats& stats() const { return m_stats; }

    /** @brief 重置统计信息 */
    void resetStatistics();

signals:
    /** @brief 求解完成信号 @param sz 矩阵尺寸 @param iterations 迭代次数 */
    void solveCompleted(int sz, int iterations);

private:
    Stats  m_stats;
    double m_timeSum;
};

#endif // TRIDIAGEIGENSOLVER_H
