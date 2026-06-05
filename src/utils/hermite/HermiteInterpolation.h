/**
 * @file HermiteInterpolation.h
 * @brief Hermite插值 — 匹配值和导数的插值
 *
 * 功能: 实现Hermite插值，同时匹配节点处的函数值和导数值，
 *       支持分段三次Hermite插值。
 *
 * 协作: BernsteinPolynomial(Bezier) / TaylorSeries(局部近似)
 */
#ifndef HERMITEINTERPOLATION_H
#define HERMITEINTERPOLATION_H

#include <QObject>
#include <QVector>

/**
 * @brief Hermite插值器
 */
class HermiteInterpolation : public QObject {
    Q_OBJECT

public:
    /** @brief 统计 */
    struct Stats {
        quint64 totalInterpolations = 0;    ///< 累计插值次数
        double  avgProcessingTimeMs = 0.0;  ///< 平均处理耗时(ms)
    };

    explicit HermiteInterpolation(QObject* parent = nullptr);

    /**
     * @brief 多点Hermite插值
     * @param x 节点x坐标
     * @param y 节点函数值
     * @param dy 节点导数值
     * @param xQuery 查询点列表
     * @return 插值结果列表
     */
    QVector<double> interpolate(const QVector<double>& x,
                                const QVector<double>& y,
                                const QVector<double>& dy,
                                const QVector<double>& xQuery);

    /**
     * @brief 单区间三次Hermite插值
     * @param x0 左端点x
     * @param y0 左端点值
     * @param d0 左端点导数
     * @param x1 右端点x
     * @param y1 右端点值
     * @param d1 右端点导数
     * @param t 参数[0,1]
     * @return 插值结果
     */
    double cubicHermite(double x0, double y0, double d0,
                        double x1, double y1, double d1,
                        double t) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 插值完成 @param count 查询点数 */
    void interpolationCompleted(int count);

private:
    Stats m_stats;                   ///< 统计信息
    double m_timeSum = 0.0;          ///< 累计耗时
};

#endif // HERMITEINTERPOLATION_H
