/**
 * @file GradientDescent2.h
 * @brief L-BFGS拟牛顿优化器 — 有限内存BFGS + Wolfe线搜索
 *
 * 功能: 实现L-BFGS (Limited-memory BFGS) 拟牛顿优化算法，
 *       支持高维无约束优化问题，使用Wolfe条件线搜索保证收敛。
 *       适用于信号处理参数估计、滤波器系数优化等场景。
 *
 * 协作: AdaptiveFilter(自适应滤波) / KalmanFilter(参数估计)
 */
#pragma once

#include <QObject>
#include <QVector>

#include <functional>
#include <vector>

/**
 * @brief L-BFGS拟牛顿优化器
 *
 * 使用有限内存BFGS近似Hessian矩阵的逆，通过双循环递归
 * 计算搜索方向，配合Wolfe条件线搜索实现高效无约束优化。
 */
class GradientDescent2 : public QObject {
    Q_OBJECT

public:
    /** @brief 优化结果结构 */
    struct Result {
        std::vector<double> x;          ///< 最优解
        double functionValue = 0.0;     ///< 最优函数值
        double gradientNorm = 0.0;      ///< 最终梯度范数
        int iterations = 0;             ///< 迭代次数
        int functionEvals = 0;          ///< 函数计算次数
        bool converged = false;         ///< 是否收敛
    };

    /** @brief 统计信息 */
    struct Stats {
        int totalOptimizations = 0;     ///< 累计优化次数
        int totalIterations = 0;        ///< 累计迭代次数
        int totalFunctionEvals = 0;     ///< 累计函数计算次数
        int totalConverged = 0;         ///< 累计收敛次数
        double avgProcessingTimeMs = 0.0; ///< 平均处理时间(ms)
    };

    /** @brief 目标函数类型: 输入向量 -> (函数值, 梯度) */
    using ObjectiveFunc = std::function<double(const std::vector<double>&, std::vector<double>&)>;

    /**
     * @brief 构造函数
     * @param memorySize L-BFGS记忆大小(存储的s/y对数)
     * @param parent 父对象
     */
    explicit GradientDescent2(int memorySize = 10, QObject* parent = nullptr);

    /**
     * @brief 执行L-BFGS优化
     * @param func 目标函数(返回函数值，通过引用返回梯度)
     * @param x0 初始点
     * @param maxIter 最大迭代次数
     * @param gradTol 梯度容差
     * @return 优化结果
     */
    Result optimize(const ObjectiveFunc& func, const std::vector<double>& x0,
                    int maxIter = 1000, double gradTol = 1e-6);

    /**
     * @brief 设置线搜索参数
     * @param c1 Wolfe条件c1(充分下降)
     * @param c2 Wolfe条件c2(曲率条件)
     */
    void setWolfeParams(double c1, double c2);

    /** @brief 获取统计信息 */
    const Stats& stats() const { return m_stats; }

    /** @brief 重置统计信息 */
    void resetStatistics();

signals:
    /** @brief 优化完成 @param result 优化结果 */
    void optimizationFinished(const Result& result);

    /** @brief 迭代进度 @param iter 当前迭代 @param fval 函数值 @param gnorm 梯度范数 */
    void iterationProgress(int iter, double fval, double gnorm);

private:
    /**
     * @brief L-BFGS双循环递归计算搜索方向
     * @param grad 当前梯度
     * @param sList 历史s向量
     * @param yList 历史y向量
     * @param rhoList 历史rho值
     * @return 搜索方向
     */
    std::vector<double> twoLoopRecursion(const std::vector<double>& grad,
                                         const QVector<std::vector<double>>& sList,
                                         const QVector<std::vector<double>>& yList,
                                         const QVector<double>& rhoList) const;

    /**
     * @brief Wolfe线搜索
     * @param func 目标函数
     * @param x 当前点
     * @param dir 搜索方向
     * @param grad 当前梯度
     * @param fVal 当前函数值
     * @return 步长alpha
     */
    double wolfeLineSearch(const ObjectiveFunc& func,
                           const std::vector<double>& x,
                           const std::vector<double>& dir,
                           const std::vector<double>& grad,
                           double fVal);

    int m_memorySize;               ///< L-BFGS记忆大小
    double m_wolfeC1 = 1e-4;        ///< Wolfe充分下降参数
    double m_wolfeC2 = 0.9;         ///< Wolfe曲率条件参数
    double m_maxStep = 1.0;         ///< 最大初始步长

    Stats m_stats;                  ///< 统计信息
    double m_timeSum = 0.0;         ///< 累计耗时
};
