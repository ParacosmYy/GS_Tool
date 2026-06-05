/**
 * @file SchurDecomp3.h
 * @brief Schur分解增强 — 实Schur/复Schur/QR迭代/特征向量恢复
 *
 * 矩阵Schur分解的完整实现:
 *   - 实Schur分解: A = Q T Q^T (T为上拟三角矩阵)
 *   - 复Schur分解: A = Q T Q^H (T为上三角矩阵)
 *   - 双位移QR迭代: 收敛速度优于单位移
 *   - 特征向量恢复: 从Schur形式反推特征向量
 * 统计分解操作次数、QR迭代次数和收敛状态。
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QElapsedTimer>
#include <QPair>

/**
 * @brief Schur分解增强
 */
class SchurDecomp3 : public QObject {
    Q_OBJECT
public:
    /** 统计信息 */
    struct Stats {
        quint64 totalDecompositions = 0; ///< 总分解次数
        quint64 totalQrIterations = 0;   ///< 总QR迭代次数
        quint64 totalEigenvectors = 0;   ///< 总特征向量恢复次数
        double avgProcessingTimeMs = 0.0;///< 平均处理耗时(ms)
    };

    /** 分解结果 */
    struct Result {
        QVector<double> T;    ///< Schur形式(行优先n*n)
        QVector<double> Q;    ///< 正交变换矩阵(行优先n*n)
        QVector<double> eigenvaluesReal; ///< 特征值实部
        QVector<double> eigenvaluesImag; ///< 特征值虚部
        bool converged;       ///< 是否收敛
        int iterations;       ///< 迭代次数
    };

    /**
     * @brief 构造函数
     * @param maxIterations QR迭代最大次数
     * @param tolerance 收敛容差
     * @param parent 父对象
     */
    explicit SchurDecomp3(int maxIterations = 300, double tolerance = 1e-12,
                          QObject* parent = nullptr);

    /**
     * @brief 实Schur分解 A = Q T Q^T
     * @param matrix 输入矩阵(行优先n*n)
     * @param n 矩阵维度
     * @return 分解结果
     */
    Result decomposeReal(const QVector<double>& matrix, int n);

    /**
     * @brief 复Schur分解 A = Q T Q^H
     * @param matrixReal 实部(行优先n*n)
     * @param matrixImag 虚部(行优先n*n)
     * @param n 矩阵维度
     * @return 分解结果(实Schur形式带2x2块)
     */
    Result decomposeComplex(const QVector<double>& matrixReal,
                            const QVector<double>& matrixImag, int n);

    /**
     * @brief 从Schur形式恢复特征向量
     * @param result Schur分解结果
     * @param n 矩阵维度
     * @return 特征向量矩阵(列向量, 行优先n*n)
     */
    QVector<double> recoverEigenvectors(const Result& result, int n);

    /** @brief 获取统计 */
    const Stats& stats() const { return m_stats; }
    /** @brief 重置统计 */
    void resetStatistics();

    /** @brief 设置最大迭代次数 */
    void setMaxIterations(int iter) { m_maxIter = iter; }
    /** @brief 设置收敛容差 */
    void setTolerance(double tol) { m_tol = tol; }

signals:
    /** 分解完成 */
    void decompositionComplete(int n, int iterations, bool converged);
    /** 特征向量恢复完成 */
    void eigenvectorsRecovered(int n);

private:
    /** Hessenberg化简(减少QR迭代工作量) */
    void hessenbergReduce(QVector<double>& H, QVector<double>& Q, int n);
    /** 单步QR迭代(双位移Francis) */
    void francisQrStep(QVector<double>& H, QVector<double>& Q, int n,
                       int lo, int hi);
    /** 计算2x2块的特征值 */
    QPair<double, double> blockEigenvalues(const QVector<double>& H, int n,
                                           int i) const;
    /** 回代求Schur形式的特征向量 */
    QVector<double> triangularEigenvectors(const QVector<double>& T,
                                           int n) const;

    int m_maxIter;
    double m_tol;
    Stats m_stats;
    double m_timeSum = 0.0;
    QElapsedTimer m_timing;
};
