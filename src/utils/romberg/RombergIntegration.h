/**
 * @file RombergIntegration.h
 * @brief Romberg数值积分 — Richardson外推加速梯形法
 *
 * 功能: 使用Romberg方法对梯形公式进行Richardson外推，
 *       以指数级收敛速度计算定积分。
 *
 * 协作: GaussLegendre(高斯求积) / RichardsonExtrapolation(Richardson外推)
 */
#ifndef ROMBERGINTEGRATION_H
#define ROMBERGINTEGRATION_H

#include <QObject>
#include <QVector>
#include <functional>

/**
 * @brief Romberg数值积分
 */
class RombergIntegration : public QObject {
    Q_OBJECT

public:
    /** @brief 一元函数类型 */
    using Func = std::function<double(double)>;

    /** @brief 统计 */
    struct Stats {
        quint64 totalIntegrations = 0;      ///< 累计积分次数
        double  avgProcessingTimeMs = 0.0;  ///< 平均处理耗时(ms)
    };

    explicit RombergIntegration(QObject* parent = nullptr);

    /**
     * @brief Romberg积分
     * @param f 被积函数
     * @param a 积分下限
     * @param b 积分上限
     * @param maxLevels 最大外推层数
     * @return 积分近似值
     */
    double integrate(Func f, double a, double b, int maxLevels = 10);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 积分完成 @param result 积分结果 @param levels 外推层数 */
    void integrationCompleted(double result, int levels);

private:
    Stats m_stats;              ///< 统计信息
    double m_timeSum = 0.0;     ///< 累计耗时
};

#endif // ROMBERGINTEGRATION_H
