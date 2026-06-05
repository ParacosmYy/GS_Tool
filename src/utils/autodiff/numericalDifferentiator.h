/**
 * @file numericalDifferentiator.h
 * @brief 数值微分引擎 — 前向/后向/中心/高阶差分
 *
 * 功能: 提供多种数值微分方法，包括前向差分、后向差分、中心差分
 *       和二阶中心差分。适用于无法获得解析导数时的数值求导。
 *
 * 协作: NewtonRaphson(数值导数根求解) / RichardsonExtrapolation(精度提升)
 */
#ifndef NUMERICALDIFFERENTIATOR_H
#define NUMERICALDIFFERENTIATOR_H

#include <QObject>
#include <functional>

/**
 * @brief 数值微分引擎
 */
class NumericalDifferentiator : public QObject {
    Q_OBJECT

public:
    /** @brief 统计信息 */
    struct Stats {
        quint64 totalDifferentiations = 0;  ///< 累计微分次数
        double  avgProcessingTimeMs = 0.0;  ///< 平均处理耗时(ms)
    };

    explicit NumericalDifferentiator(QObject* parent = nullptr);

    /**
     * @brief 前向差分求一阶导数
     * @param f 目标函数
     * @param x 求导点
     * @param h 步长(默认1e-6)
     * @return f'(x)的近似值
     *
     * 公式: f'(x) ≈ (f(x+h) - f(x)) / h
     * 精度: O(h)，截断误差与舍入误差的平衡点在h≈√ε
     */
    double forwardDifference(std::function<double(double)> f,
                             double x, double h = 1e-6);

    /**
     * @brief 后向差分求一阶导数
     * @param f 目标函数
     * @param x 求导点
     * @param h 步长(默认1e-6)
     * @return f'(x)的近似值
     *
     * 公式: f'(x) ≈ (f(x) - f(x-h)) / h
     * 精度: O(h)
     */
    double backwardDifference(std::function<double(double)> f,
                              double x, double h = 1e-6);

    /**
     * @brief 中心差分求一阶导数
     * @param f 目标函数
     * @param x 求导点
     * @param h 步长(默认1e-6)
     * @return f'(x)的近似值
     *
     * 公式: f'(x) ≈ (f(x+h) - f(x-h)) / (2h)
     * 精度: O(h²)，比前向/后向差分更精确
     */
    double centralDifference(std::function<double(double)> f,
                             double x, double h = 1e-6);

    /**
     * @brief 中心差分求二阶导数
     * @param f 目标函数
     * @param x 求导点
     * @param h 步长(默认1e-5)
     * @return f''(x)的近似值
     *
     * 公式: f''(x) ≈ (f(x+h) - 2f(x) + f(x-h)) / h²
     * 精度: O(h²)
     */
    double secondDerivative(std::function<double(double)> f,
                            double x, double h = 1e-5);

    /** @brief 获取统计信息 */
    const Stats& stats() const { return m_stats; }

    /** @brief 重置统计信息 */
    void resetStatistics();

signals:
    /** @brief 微分完成信号 @param result 导数值 */
    void differentiationCompleted(double result);

private:
    Stats m_stats;              ///< 统计信息
    double m_timeSumMs;         ///< 累计耗时(ms)
};

#endif // NUMERICALDIFFERENTIATOR_H
