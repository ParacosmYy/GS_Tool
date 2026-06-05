/**
 * @file BfgsOptimizer.h
 * @brief BFGS拟牛顿优化器 — 无约束非线性优化
 *
 * 功能: 使用BFGS(Broyden-Fletcher-Goldfarb-Shanno)算法求解
 *       无约束非线性优化问题。维护Hessian矩阵的近似逆。
 *
 * 协作: SimulatedAnnealing(全局) / GeneticOptimizer(进化)
 */
#ifndef BFGSOPTIMIZER_H
#define BFGSOPTIMIZER_H

#include <QObject>
#include <QVector>
#include <functional>

/**
 * @brief BFGS拟牛顿优化器
 */
class BfgsOptimizer : public QObject {
    Q_OBJECT

public:
    /** @brief 目标函数 */
    using ObjFunc = std::function<double(const QVector<double>&)>;

    /** @brief 梯度函数 */
    using GradFunc = std::function<QVector<double>(const QVector<double>&)>;

    /** @brief 统计信息 */
    struct Stats {
        quint64 totalIterations = 0;    ///< 累计迭代次数
        double avgProcessingTimeMs = 0.0; ///< 平均处理时间(ms)
    };

    /** @brief 优化结果 */
    struct Result {
        QVector<double> optimalPoint;   ///< 最优点
        double optimalValue = 0.0;      ///< 最优值
        int iterations = 0;             ///< 实际迭代次数
        bool converged = false;         ///< 是否收敛
    };

    explicit BfgsOptimizer(QObject* parent = nullptr);

    /** @brief 执行BFGS优化
     *  @param objective 目标函数
     *  @param gradient 梯度函数(空则数值差分)
     *  @param initialPoint 初始点
     *  @param maxIter 最大迭代次数
     *  @param tol 收敛容限
     *  @return 优化结果 */
    Result optimize(const ObjFunc& objective,
                    const GradFunc& gradient,
                    const QVector<double>& initialPoint,
                    int maxIter = 500,
                    double tol = 1e-8);

    /** @brief 获取统计 */
    const Stats& stats() const { return m_stats; }

    /** @brief 重置统计 */
    void resetStatistics();

signals:
    /** @brief 迭代完成 @param iter 迭代号 @param value 当前值 */
    void iterationCompleted(int iter, double value);

    /** @brief 优化完成 @param value 最优值 */
    void optimizationCompleted(double value);

private:
    /** @brief 数值梯度 */
    QVector<double> numericalGradient(const ObjFunc& obj,
                                      const QVector<double>& x,
                                      double eps = 1e-6);

    /** @brief 线搜索(回溯Armijo) */
    double lineSearch(const ObjFunc& obj,
                      const QVector<double>& x,
                      const QVector<double>& dir,
                      const QVector<double>& grad);

    double m_timeSum;   ///< 处理时间累加器
    Stats m_stats;      ///< 统计信息
};

#endif // BFGSOPTIMIZER_H
