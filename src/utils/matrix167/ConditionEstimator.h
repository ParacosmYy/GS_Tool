/**
 * @file ConditionEstimator.h
 * @brief 矩阵条件数估计(Hager 1-范数法) — Matrix Condition Number Estimation via Hager's 1-Norm Method
 *
 * 功能: 实现Hager算法估计矩阵1-范数条件数。
 *       通过迭代求解 A^T * sign(A*x) 来逼近 ||A||_1 * ||A^{-1}||_1。
 *       支持LU分解后的条件数估计和奇异值分解精确计算。
 *
 * 协作: MatrixDecomposition(LU分解) / SvdSolver(SVD) / LinearSolver(线性求解)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 矩阵条件数估计器
 */
class ConditionEstimator : public QObject {
    Q_OBJECT

public:
    /** @brief 估计方法 */
    enum class Method {
        Hager1Norm,     ///< Hager迭代估计1-范数条件数
        SVDFull         ///< SVD精确计算2-范数条件数
    };

    /** @brief 运行统计 */
    struct Stats {
        quint64 totalEstimates = 0;         ///< 累计估计次数
        quint64 hagerIterations = 0;        ///< Hager迭代总次数
        double lastConditionNumber = 0.0;   ///< 最近一次条件数
        double avgProcessingTimeMs = 0.0;   ///< 平均处理耗时(ms)
    };

    explicit ConditionEstimator(QObject* parent = nullptr);
    ~ConditionEstimator() override;

    /**
     * @brief 设置估计方法
     * @param method 估计方法
     */
    void setMethod(Method method);

    /**
     * @brief 设置最大迭代次数(Hager方法)
     * @param maxIter 最大迭代次数
     */
    void setMaxIterations(int maxIter);

    /**
     * @brief 估计矩阵条件数(需要LU分解后的矩阵)
     * @param matrix 输入方阵(row-major)
     * @return 估计的条件数kappa
     */
    double estimate(const QVector<QVector<double>>& matrix);

    /**
     * @brief 通过LU分解估计条件数
     * @param lu LU分解后的矩阵(下三角L + 上三角U)
     * @param perm 行置换数组
     * @return 估计的条件数
     */
    double estimateFromLU(const QVector<QVector<double>>& lu,
                           const QVector<int>& perm);

    /**
     * @brief 精确计算SVD条件数(2-范数)
     * @param singularValues 奇异值(降序排列)
     * @return 条件数 = sigma_max / sigma_min
     */
    double conditionFromSVD(const QVector<double>& singularValues) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 估计完成 @param condition 条件数 @param iterations 迭代次数 */
    void estimateCompleted(double condition, int iterations);

private:
    /** @brief Hager算法：估计矩阵1-范数||A^{-1}||_1 */
    double hagerNorm(const QVector<QVector<double>>& lu,
                      const QVector<int>& perm);

    /** @brief 前向替换(Ly = Pb) */
    static QVector<double> forwardSolve(const QVector<QVector<double>>& lu,
                                         const QVector<int>& perm,
                                         const QVector<double>& b);

    /** @brief 后向替换(Ux = y) */
    static QVector<double> backwardSolve(const QVector<QVector<double>>& lu,
                                          const QVector<double>& y);

    /** @brief 计算矩阵1-范数(最大列和) */
    static double norm1(const QVector<QVector<double>>& matrix);

    Method m_method = Method::Hager1Norm;
    int m_maxIterations = 100;

    Stats m_stats;
    double m_timeSum = 0.0;
};
