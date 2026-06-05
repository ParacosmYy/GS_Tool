/**
 * @file CatmullRomSpline.h
 * @brief Catmull-Rom样条(Catmull-Rom Spline)
 */

#pragma once

#include <QObject>
#include <QVector>

/**
 * @class CatmullRomSpline
 * @brief Catmull-Rom样条 — 经过所有控制点的插值样条
 *
 * 支持开放/闭合样条、均匀/向心参数化。
 * 适用于动画路径、曲线绘制、数据平滑等场景。
 */
class CatmullRomSpline : public QObject
{
    Q_OBJECT

public:
    /** @brief 参数化类型 */
    enum Parameterization {
        Uniform,    /**< 均匀参数化 */
        Centripetal /**< 向心参数化 */
    };
    Q_ENUM(Parameterization)

    /** @brief 统计信息 */
    struct Stats {
        int totalEvaluated = 0; /**< 总评估次数 */
        int totalPoints = 0;    /**< 总控制点数 */
        double avgProcessingTimeMs = 0.0; /**< 平均处理时间(ms) */
    };

    /** @brief 构造函数 */
    explicit CatmullRomSpline(QObject* parent = nullptr);

    /**
     * @brief 设置控制点
     * @param x X坐标列表
     * @param y Y坐标列表
     * @param closed 是否闭合
     * @param param 参数化类型
     */
    void setPoints(const QVector<double>& x, const QVector<double>& y,
                    bool closed = false,
                    Parameterization param = Centripetal);

    /**
     * @brief 在参数t处评估
     * @param t 参数[0, segments]
     * @return (x, y)坐标
     */
    QPair<double, double> evaluate(double t) const;

    /**
     * @brief 均匀采样
     * @param nSamples 每段采样数
     * @return 采样点列表(x, y)
     */
    QVector<QPair<double, double>> sample(int nSamples = 20) const;

    /**
     * @brief 段数
     */
    int segments() const;

    /** @brief 获取统计 */
    Stats stats() const;

    /** @brief 重置统计 */
    void resetStatistics();

signals:
    /** @brief 评估完成信号 */
    void evaluated(int pointCount);

private:
    QVector<double> m_x, m_y;
    QVector<double> m_knots;
    bool m_closed;
    Parameterization m_param;

    Stats m_stats;
    double m_timeSum;
};
