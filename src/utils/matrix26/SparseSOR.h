/**
 * @file SparseSOR.h
 * @brief 稀疏SOR迭代求解器 — 逐次超松弛/稀疏矩阵向量乘/动态omega调节
 *
 * 功能: 实现稀疏矩阵的逐次超松弛(SOR)迭代求解，支持CSR格式存储、
 *       动态松弛因子调节、收敛监测和残差计算。
 *
 * 协作: DataTransformer(数据变换) / DigitalFilter(数字滤波器)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 稀疏SOR迭代求解器
 */
class SparseSOR : public QObject {
    Q_OBJECT

public:
    /** @brief 统计 */
    struct Stats {
        quint64 totalSolves = 0;               ///< 累计求解次数
        quint64 totalIterations = 0;           ///< 累计迭代次数
        double  avgProcessingTimeMs = 0.0;     ///< 平均处理时间(ms)
        double  avgIterations = 0.0;           ///< 平均迭代次数
        double  bestResidual = 1e30;           ///< 历史最小残差
    };

    /** @brief 求解结果 */
    struct SolveResult {
        QVector<double> solution;    ///< 解向量
        double finalResidual = 0.0;  ///< 最终残差
        int iterations = 0;          ///< 实际迭代次数
        bool converged = false;      ///< 是否收敛
        double omegaUsed = 0.0;      ///< 使用的omega
    };

    explicit SparseSOR(QObject* parent = nullptr);

    /** @brief 从COO格式构建CSR矩阵 @param rows 行索引 @param cols 列索引 @param vals 值 @param n 矩阵维度 */
    void buildFromCOO(const QVector<int>& rows, const QVector<int>& cols,
                      const QVector<double>& vals, int n);

    /** @brief 设置松弛因子 @param omega 松弛因子(0<omega<2) */
    void setOmega(double omega);

    /** @brief 启用/禁用动态omega调节 @param enable 是否启用 */
    void setDynamicOmega(bool enable);

    /** @brief 设置收敛容差 @param tol 容差 */
    void setTolerance(double tol);

    /** @brief 设置最大迭代次数 @param maxIter 最大迭代 */
    void setMaxIterations(int maxIter);

    /** @brief 求解Ax=b @param rhs 右端向量 @return 求解结果 */
    SolveResult solve(const QVector<double>& rhs);

    /** @brief 求解(指定初始猜测) @param rhs 右端向量 @param initialGuess 初始解 @return 求解结果 */
    SolveResult solveWithGuess(const QVector<double>& rhs,
                               const QVector<double>& initialGuess);

    /** @brief 计算残差 r=b-Ax @param x 解向量 @param rhs 右端向量 @return 残差向量 */
    QVector<double> residual(const QVector<double>& x,
                             const QVector<double>& rhs) const;

    /** @brief 稀疏矩阵向量乘 y=Ax @param x 输入向量 @return 结果向量 */
    QVector<double> spmv(const QVector<double>& x) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 求解完成 @param converged 是否收敛 @param iterations 迭代次数 */
    void solveComplete(bool converged, int iterations);

    /** @brief 迭代进展 @param iteration 当前迭代 @param residual 当前残差 */
    void iterationProgress(int iteration, double residual);

private:
    double computeResidualNorm(const QVector<double>& x,
                               const QVector<double>& rhs) const;
    double adjustOmega(int iteration, double oldResidual, double newResidual);

    int m_n;                       ///< 矩阵维度
    double m_omega;                ///< 松弛因子
    bool m_dynamicOmega;           ///< 动态omega调节
    double m_tolerance;            ///< 收敛容差
    int m_maxIter;                 ///< 最大迭代次数

    /* CSR格式存储 */
    QVector<int> m_rowPtr;         ///< 行指针
    QVector<int> m_colIdx;         ///< 列索引
    QVector<double> m_values;      ///< 非零值
    QVector<double> m_diag;        ///< 对角线元素

    Stats m_stats;
    double m_timeSum = 0.0;        ///< 处理时间累加器
};
