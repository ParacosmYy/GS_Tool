/**
 * @file CircadianRhythm.h
 * @brief 昼夜节律分析器(Circadian Rhythm Analyzer)
 */

#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @class CircadianRhythm
 * @brief 昼夜节律分析器 — 检测和分析周期性生物/信号节律
 *
 * 支持余弦拟合、周期检测、相位分析。
 * 适用于生理信号分析、环境监测、时钟恢复等场景。
 */
class CircadianRhythm : public QObject
{
    Q_OBJECT

public:
    /** @brief 拟合结果 */
    struct FitResult {
        double amplitude = 0.0;    /**< 振幅 */
        double mesor = 0.0;        /**< 中值统计量(Midline Statistic of Rhythm) */
        double acrophase = 0.0;    /**< 峰值相位(弧度) */
        double period = 24.0;      /**< 拟合周期 */
        double rSquared = 0.0;     /**< R²拟合优度 */
    };

    /** @brief 统计信息 */
    struct Stats {
        int totalFits = 0;         /**< 总拟合次数 */
        int totalPeriods = 0;      /**< 总周期检测次数 */
        double avgProcessingTimeMs = 0.0; /**< 平均处理时间(ms) */
    };

    /** @brief 构造函数 */
    explicit CircadianRhythm(QObject* parent = nullptr);

    /**
     * @brief 余弦拟合(单周期)
     * @param timepoints 时间点列表(小时)
     * @param values 对应测量值
     * @param period 拟合周期(默认24小时)
     * @return 拟合结果
     */
    FitResult cosinor(const QVector<double>& timepoints,
                       const QVector<double>& values,
                       double period = 24.0);

    /**
     * @brief 多周期余弦拟合
     * @param timepoints 时间点列表
     * @param values 测量值列表
     * @param periods 候选周期列表
     * @return 最优拟合结果
     */
    FitResult multiCosinor(const QVector<double>& timepoints,
                            const QVector<double>& values,
                            const QVector<double>& periods);

    /**
     * @brief 周期检测(Lomb-Scargle简化)
     * @param timepoints 时间点列表
     * @param values 测量值列表
     * @param minPeriod 最小周期
     * @param maxPeriod 最大周期
     * @param steps 搜索步数
     * @return 检测到的周期
     */
    double detectPeriod(const QVector<double>& timepoints,
                         const QVector<double>& values,
                         double minPeriod, double maxPeriod, int steps = 100);

    /**
     * @brief 计算相位一致性
     * @param phases 相位列表(弧度)
     * @return 一致性指标[0,1]
     */
    static double phaseCoherence(const QVector<double>& phases);

    /**
     * @brief 计算IS(节律稳定性指数)
     * @param values 逐小时测量值
     * @return 稳定性指数[0,1]
     */
    static double interdailyStability(const QVector<double>& values);

    /**
     * @brief 计算IV(节律变异性指数)
     * @param values 逐小时测量值
     * @return 变异性指数(≥0)
     */
    static double intradailyVariability(const QVector<double>& values);

    /** @brief 获取统计 */
    Stats stats() const;

    /** @brief 重置统计 */
    void resetStatistics();

signals:
    /** @brief 拟合完成信号 */
    void fitCompleted(double period, double rSquared);

    /** @brief 周期检测完成信号 */
    void periodDetected(double period);

private:
    double powerAtPeriod(const QVector<double>& tp, const QVector<double>& val,
                          double period) const;

    Stats m_stats;
    double m_timeSum;
};
