/**
 * @file SymmetricEigen4.h
 * @brief 对称特征值增强 — Jacobi旋转/三对角化/二分法/分治
 *
 * 功能: 求解对称矩阵的全部特征值和特征向量，支持Jacobi旋转迭代、
 *       Householder三对角化+QR迭代、Sturm二分法、分治算法。
 *
 * 协作: SpectrumAnalyzer(频域) / DataClassifier(分类)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 对称特征值增强 — 多算法特征分解
 */
class SymmetricEigen4 : public QObject {
    Q_OBJECT

public:
    /** @brief 算法选择 */
    enum class Algorithm {
        Jacobi,         ///< Jacobi旋转(精确, 适合小矩阵)
        TridiagonalQR,  ///< 三对角化+隐式QR(通用)
        Bisection,      ///< Sturm二分法(求指定范围特征值)
        DivideConquer   ///< 分治法(适合大规模)
    };
    Q_ENUM(Algorithm)

    /** @brief 特征分解结果 */
    struct EigenResult {
        QVector<double> eigenvalues;             ///< 特征值(升序)
        QVector<QVector<double>> eigenvectors;   ///< 特征向量(列)
        int iterations = 0;                      ///< 迭代次数
        bool converged = false;                  ///< 是否收敛
    };

    /** @brief 统计 */
    struct Stats {
        quint64 totalDecompositions = 0;       ///< 累计分解次数
        quint64 totalElementsProcessed = 0;    ///< 累计处理矩阵元素数
        double  avgProcessingTimeMs = 0.0;     ///< 平均处理时间(ms)
        quint64 totalConverged = 0;            ///< 收敛次数
    };

    explicit SymmetricEigen4(QObject* parent = nullptr);

    /** @brief 设置收敛阈值 @param tol 容差(默认1e-10) */
    void setTolerance(double tol);

    /** @brief 设置最大迭代次数 @param maxIter 最大迭代 */
    void setMaxIterations(int maxIter);

    /** @brief 特征分解 @param matrix 对称矩阵(行优先, n*n) @param n 矩阵阶数 @param algo 算法 @return 结果 */
    EigenResult decompose(const QVector<double>& matrix, int n,
                          Algorithm algo = Algorithm::TridiagonalQR);

    /** @brief 三对角化(Householder) @param matrix 对称矩阵 @param n 阶数 @return (对角线, 次对角线, 变换矩阵) */
    void tridiagonalize(const QVector<double>& matrix, int n,
                        QVector<double>& diagonal,
                        QVector<double>& subdiagonal,
                        QVector<double>& transform);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 分解完成 @param n 矩阵阶数 @param converged 是否收敛 */
    void decomposeComplete(int n, bool converged);

private:
    EigenResult jacobiMethod(const QVector<double>& matrix, int n);
    EigenResult tridiagQRMethod(const QVector<double>& matrix, int n);
    EigenResult bisectionMethod(const QVector<double>& matrix, int n);
    EigenResult divideConquerMethod(const QVector<double>& matrix, int n);

    void implicitQLShift(QVector<double>& diag, QVector<double>& subdiag,
                         QVector<double>& transform, int n);
    int sturmCount(const QVector<double>& diag,
                   const QVector<double>& subdiag, double x, int n) const;

    double m_tolerance = 1e-10;      ///< 收敛容差
    int m_maxIterations = 1000;      ///< 最大迭代次数

    Stats m_stats;
    double m_timeSum = 0.0;
};
