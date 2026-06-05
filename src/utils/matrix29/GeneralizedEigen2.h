/**
 * @file GeneralizedEigen2.h
 * @brief 广义特征值求解 — QZ分解/Schur分解
 *
 * 功能: 求解广义特征值问题 Ax = λBx，支持QZ分解(QZ算法)、
 *       实Schur分解、同时对角化，统计求解次数/迭代步数/平均处理耗时。
 */
#ifndef GENERALIZEDEIGEN2_H
#define GENERALIZEDEIGEN2_H

#include <QObject>
#include <QVector>
#include <QList>
#include <QPair>
#include <QComplex>

/**
 * @class GeneralizedEigen2
 * @brief 广义特征值求解器，基于QZ分解
 */
class GeneralizedEigen2 : public QObject {
    Q_OBJECT
public:
    /** 特征值 */
    struct Eigenvalue {
        double real;        ///< 实部
        double imag;        ///< 虚部
        bool isFinite;      ///< 是否有限值(B矩阵非奇异)
    };

    /** 广义Schur分解结果 */
    struct SchurResult {
        QVector<QVector<double>> Q;     ///< 左正交矩阵
        QVector<QVector<double>> Z;     ///< 右正交矩阵
        QVector<QVector<double>> S;     ///< 上三角Schur矩阵S
        QVector<QVector<double>> T;     ///< 上三角Schur矩阵T
        QList<Eigenvalue> eigenvalues;  ///< 广义特征值
    };

    /** 求解结果 */
    struct SolveResult {
        QList<Eigenvalue> eigenvalues;          ///< 广义特征值
        QVector<QVector<double>> eigenvectors;  ///< 右特征向量
        int iterations;                         ///< 迭代步数
        double residual;                        ///< 残差范数
    };

    /** 统计信息 */
    struct Stats {
        quint64 totalSolves = 0;                ///< 总求解次数
        quint64 totalIterations = 0;            ///< 累计迭代步数
        double  avgIterations = 0.0;            ///< 平均迭代步数
        double  avgProcessingTimeMs = 0.0;      ///< 平均处理耗时(ms)
    };

    explicit GeneralizedEigen2(QObject* parent = nullptr);

    /** 设置参数 */
    void setMaxIterations(int maxIter);
    void setTolerance(double tol);

    /** 求解广义特征值问题 Ax = λBx */
    SolveResult solve(const QVector<QVector<double>>& A,
                      const QVector<QVector<double>>& B);

    /** QZ分解(广义Schur分解) */
    SchurResult qzDecompose(const QVector<QVector<double>>& A,
                            const QVector<QVector<double>>& B);

    /** 验证结果: 计算||A·v - λ·B·v|| */
    double computeResidual(const QVector<QVector<double>>& A,
                           const QVector<QVector<double>>& B,
                           const Eigenvalue& lambda,
                           const QVector<double>& v) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** 求解完成信号 */
    void solveComplete(int eigenvalueCount, int iterations);
    /** 迭代步信号 */
    void iterationStep(int step, double shift);

private:
    /** Hessenberg化简 */
    void hessenbergReduce(QVector<QVector<double>>& A,
                          QVector<QVector<double>>& B,
                          QVector<QVector<double>>& Q,
                          QVector<QVector<double>>& Z);
    /** QZ步(隐式位移) */
    void qzStep(QVector<QVector<double>>& A, QVector<QVector<double>>& B,
                QVector<QVector<double>>& Q, QVector<QVector<double>>& Z,
                int lo, int hi);
    /** Householder反射 */
    void householder(QVector<double>& v, double& beta, int size) const;
    /** 从Schur形式提取特征值 */
    QList<Eigenvalue> extractEigenvalues(const QVector<QVector<double>>& S,
                                         const QVector<QVector<double>>& T) const;
    /** 计算矩阵范数 */
    double matrixNorm(const QVector<QVector<double>>& M) const;

    int m_maxIterations;
    double m_tolerance;
    Stats m_stats;
    double m_timeSum;
};

#endif // GENERALIZEDEIGEN2_H
