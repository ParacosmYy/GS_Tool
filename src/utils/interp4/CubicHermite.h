/**
 * @file CubicHermite.h
 * @brief 三次Hermite插值(Cubic Hermite Interpolation)
 */

#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @class CubicHermite
 * @brief 三次Hermite插值 — 保形分段三次插值
 *
 * 支持单调性保持(Fritsch-Carlson方法)、边界导数设置。
 * 适用于信号重建、曲线绘制、数据插值等场景。
 */
class CubicHermite : public QObject
{
    Q_OBJECT

public:
    /** @brief 统计信息 */
    struct Stats {
        int totalInterpolated = 0; /**< 总插值次数 */
        int totalPoints = 0;       /**< 总数据点数 */
        double avgProcessingTimeMs = 0.0; /**< 平均处理时间(ms) */
    };

    /** @brief 构造函数 */
    explicit CubicHermite(QObject* parent = nullptr);

    /**
     * @brief 设置数据点
     * @param x X坐标(必须递增)
     * @param y Y坐标
     * @param slopes 各点斜率(空则自动计算)
     */
    void setData(const QVector<double>& x, const QVector<double>& y,
                  QVector<double> slopes = {});

    /**
     * @brief 单点插值
     * @param x 插值点
     * @return 插值结果
     */
    double interpolate(double x) const;

    /**
     * @brief 批量插值
     * @param xPoints 插值点列表
     * @return 插值结果列表
     */
    QVector<double> interpolateBatch(const QVector<double>& xPoints) const;

    /**
     * @brief 自动计算保形单调斜率(Fritsch-Carlson)
     * @param x X坐标
     * @param y Y坐标
     * @return 各点斜率
     */
    static QVector<double> monotoneSlopes(const QVector<double>& x,
                                            const QVector<double>& y);

    /**
     * @brief Hermite基函数值
     * @param t 参数[0,1]
     * @param p0 起点值
     * @param p1 终点值
     * @param m0 起点切线
     * @param m1 终点切线
     * @return 插值结果
     */
    static double hermite(double t, double p0, double p1,
                           double m0, double m1);

    /** @brief 获取统计 */
    Stats stats() const;

    /** @brief 重置统计 */
    void resetStatistics();

signals:
    /** @brief 插值完成信号 */
    void interpolationCompleted(int pointCount);

private:
    int findSegment(double x) const;

    QVector<double> m_x;
    QVector<double> m_y;
    QVector<double> m_slopes;
    Stats m_stats;
    double m_timeSum;
};
