/**
 * @file NumericalIntegrator.h
 * @brief 数值积分引擎 — 梯形/辛普森/矩形/龙贝格积分
 *
 * 功能: 4种数值积分方法，支持等间距和非等间距数据，
 *       计算定积分值和累计积分曲线。
 *
 * 协作: NumericalDerivative(微分) / WaveformFilter(预处理)
 */
#ifndef NUMERICALINTEGRATOR_H
#define NUMERICALINTEGRATOR_H

#include <QObject>
#include <QVector>

class NumericalIntegrator : public QObject {
    Q_OBJECT

public:
    /** @brief 积分方法 */
    enum class Method {
        Rectangle,     ///< 矩形法(左端点)
        Trapezoidal,   ///< 梯形法
        Simpson,       ///< 辛普森法(抛物线)
        Romberg        ///< 龙贝格外推
    };
    Q_ENUM(Method)

    /** @brief 统计 */
    struct Stats {
        quint64 totalIntegrations = 0;      ///< 累计积分次数
        quint64 totalPointsProcessed = 0;   ///< 累计处理点数
        double  peakValue = 0.0;            ///< 峰值积分值
        double  averageValue = 0.0;         ///< 平均积分值
    };

    explicit NumericalIntegrator(QObject* parent = nullptr);

    void setMethod(Method method);
    void setDx(double dx);

    /** @brief 计算定积分 @param data 数据 @return 积分值 */
    double integrate(const QVector<double>& data);

    /** @brief 计算累计积分曲线 @param data 数据 @return 累计积分 */
    QVector<double> cumulativeIntegrate(const QVector<double>& data);

    /** @brief 指定区间积分 @param data 数据 @param start 起始 @param end 结束 @return 积分值 */
    double integrateRange(const QVector<double>& data, int start, int end);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void integrationComplete(double value);

private:
    double integrateRectangle(const QVector<double>& data) const;
    double integrateTrapezoidal(const QVector<double>& data) const;
    double integrateSimpson(const QVector<double>& data) const;
    double integrateRomberg(const QVector<double>& data) const;

    Method m_method;
    double m_dx;
    double m_valueSum;
    Stats m_stats;
};

#endif // NUMERICALINTEGRATOR_H
