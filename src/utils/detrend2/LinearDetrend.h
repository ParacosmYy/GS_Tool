/**
 * @file LinearDetrend.h
 * @brief 线性去趋势 — 最小二乘法线性趋势估计与移除
 *
 * 功能: 对信号进行最小二乘线性拟合，然后移除线性趋势分量，
 *       使信号在均值为零附近波动。适用于数据预处理。
 *
 * 协作: DetrendEngine(通用去趋势) / PolynomialRegression(多项式)
 */
#ifndef LINEARDETREND_H
#define LINEARDETREND_H

#include <QObject>
#include <QVector>

/**
 * @brief 线性去趋势处理器
 */
class LinearDetrend : public QObject {
    Q_OBJECT

public:
    /** @brief 线性拟合结果 */
    struct LineFit {
        double slope = 0.0;     ///< 斜率
        double intercept = 0.0; ///< 截距
        double rSquared = 0.0;  ///< R²拟合优度
    };

    /** @brief 统计信息 */
    struct Stats {
        quint64 totalDetrends = 0;       ///< 累计去趋势次数
        double  avgProcessingTimeMs = 0.0; ///< 平均处理时间(ms)
    };

    explicit LinearDetrend(QObject* parent = nullptr);

    /** @brief 对信号进行线性去趋势
     *  @param signal 输入信号
     *  @return 去趋势后的信号 */
    QVector<double> detrend(const QVector<double>& signal);

    /** @brief 拟合线性趋势线
     *  @param signal 输入信号
     *  @return 拟合结果(斜率、截距、R²) */
    LineFit fitLine(const QVector<double>& signal);

    /** @brief 获取统计 */
    const Stats& stats() const { return m_stats; }

    /** @brief 重置统计 */
    void resetStatistics();

signals:
    /** @brief 去趋势完成 @param signalSize 信号长度 */
    void detrendCompleted(int signalSize);

private:
    double m_timeSum;   ///< 处理时间累加器
    Stats  m_stats;     ///< 统计信息
};

#endif // LINEARDETREND_H
