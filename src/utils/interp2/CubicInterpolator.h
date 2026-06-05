/**
 * @file CubicInterpolator.h
 * @brief 三次样条插值器 — 自然/夹紧边界条件
 *
 * 功能: 构造三次样条插值函数，支持自然边界和夹紧边界条件。
 *       用于信号重采样、曲线拟合和数据补全。
 *
 * 协作: DataResampler(重采样) / BezierSpline(贝塞尔)
 */
#ifndef CUBICINTERPOLATOR_H
#define CUBICINTERPOLATOR_H

#include <QObject>
#include <QVector>

/**
 * @brief 三次样条插值器
 */
class CubicInterpolator : public QObject {
    Q_OBJECT

public:
    /** @brief 统计信息 */
    struct Stats {
        quint64 totalInterpolations = 0;  ///< 累计插值次数
        double avgProcessingTimeMs = 0.0; ///< 平均处理时间(ms)
    };

    explicit CubicInterpolator(QObject* parent = nullptr);

    /** @brief 构建自然三次样条(二阶导边界=0)
     *  @param x 自变量(必须递增)
     *  @param y 因变量 */
    void buildNatural(const QVector<double>& x, const QVector<double>& y);

    /** @brief 构建夹紧三次样条(指定一阶导边界)
     *  @param x 自变量
     *  @param y 因变量
     *  @param dLeft 左边界一阶导
     *  @param dRight 右边界一阶导 */
    void buildClamped(const QVector<double>& x, const QVector<double>& y,
                      double dLeft, double dRight);

    /** @brief 在给定x处求值
     *  @param x 查询点
     *  @return 插值结果 */
    double evaluate(double x) const;

    /** @brief 批量求值
     *  @param xPoints 查询点数组
     *  @return 插值结果数组 */
    QVector<double> evaluateBatch(const QVector<double>& xPoints) const;

    /** @brief 获取统计 */
    const Stats& stats() const { return m_stats; }

    /** @brief 重置统计 */
    void resetStatistics();

signals:
    /** @brief 插值完成 @param count 查询点数 */
    void interpolationCompleted(int count);

private:
    /** @brief 三对角方程求解(Thomas算法) */
    QVector<double> solveTridiagonal(const QVector<double>& diag,
                                     const QVector<double>& upper,
                                     const QVector<double>& lower,
                                     const QVector<double>& rhs);

    /** @brief 查找x所在区间 */
    int findSegment(double x) const;

    QVector<double> m_x;    ///< 节点x
    QVector<double> m_a;    ///< 系数a (y值)
    QVector<double> m_b;    ///< 系数b
    QVector<double> m_c;    ///< 系数c
    QVector<double> m_d;    ///< 系数d
    bool m_built;           ///< 是否已构建

    double m_timeSum;       ///< 处理时间累加器
    Stats m_stats;          ///< 统计信息
};

#endif // CUBICINTERPOLATOR_H
