/**
 * @file BandEigenSolver.h
 * @brief 带状矩阵特征值求解器 — Banded Matrix Eigenvalue Solver
 *
 * 功能: 对对称带状矩阵执行Householder三对角化 + 隐式QR迭代，
 *       求解全部或指定范围的特征值及特征向量。
 *
 * 协作: BandMatrixSolver(带状方程求解) / EigenDecomposition(稠密特征值)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 对称带状矩阵特征值求解器
 */
class BandEigenSolver : public QObject {
    Q_OBJECT

public:
    /** @brief 运行统计 */
    struct Stats {
        quint64 totalSolves = 0;            ///< 累计求解次数
        quint64 totalIterations = 0;        ///< 累计QR迭代次数
        double avgProcessingTimeMs = 0.0;   ///< 平均处理耗时(ms)
        double convergenceRate = 0.0;       ///< 收敛率(0~1)
    };

    explicit BandEigenSolver(QObject* parent = nullptr);

    /**
     * @brief 设置最大QR迭代次数
     * @param maxIter 最大迭代次数
     */
    void setMaxIterations(int maxIter);

    /**
     * @brief 设置收敛容差
     * @param tol 对角元素收敛阈值
     */
    void setTolerance(double tol);

    /**
     * @brief 求解对称带状矩阵的全部特征值
     * @param bandMatrix 带状存储(n × bandwidth+1)，对称下半带
     * @param n 矩阵阶数
     * @param bandwidth 半带宽
     * @return 特征值向量(升序排列)
     */
    QVector<double> solveEigenvalues(const QVector<QVector<double>>& bandMatrix,
                                     int n, int bandwidth);

    /**
     * @brief 求解对称带状矩阵的特征值和特征向量
     * @param bandMatrix 带状存储
     * @param n 矩阵阶数
     * @param bandwidth 半带宽
     * @param[out] eigenvectors 特征向量矩阵(每行一个特征向量)
     * @return 特征值向量(升序排列)
     */
    QVector<double> solve(const QVector<QVector<double>>& bandMatrix,
                          int n, int bandwidth,
                          QVector<QVector<double>>& eigenvectors);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 求解完成 @param n 矩阵阶数 @param iterations 实际迭代次数 */
    void solveCompleted(int n, int iterations);

private:
    /** @brief 带状转三对角(Householder) */
    void bandToTridiag(const QVector<QVector<double>>& bandMatrix,
                       int n, int bandwidth,
                       QVector<double>& diag, QVector<double>& subdiag);

    /** @brief 三对角QR迭代求解特征值 */
    QVector<double> tridiagQR(QVector<double>& diag, QVector<double>& subdiag,
                              int n, QVector<QVector<double>>& Q);

    /** @brief Wilkinson位移 */
    double wilkinsonShift(double d, double e) const;

    int m_maxIter = 1000;
    double m_tolerance = 1e-10;

    Stats m_stats;
    double m_timeSum = 0.0;
};
