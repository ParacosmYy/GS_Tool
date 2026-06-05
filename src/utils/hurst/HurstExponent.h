/**
 * @file HurstExponent.h
 * @brief Hurst指数计算器 — R/S分析/长程依赖检测
 *
 * 功能: 经典R/S分析计算Hurst指数，支持滑动窗口分析，
 *       长程依赖性判断(H<0.5均值回复,H=0.5随机游走,H>0.5持续性)，
 *       统计分析次数/H指数/耗时。
 */
#ifndef HURSTEXPONENT_H
#define HURSTEXPONENT_H

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief Hurst指数计算器
 */
class HurstExponent : public QObject {
    Q_OBJECT

public:
    /** @brief 统计 */
    struct Stats {
        quint64 totalAnalyses = 0;    ///< 累计分析次数
        double  lastHurst = 0.0;       ///< 最近Hurst指数
        double  avgHurst = 0.0;        ///< 平均Hurst指数
        double  avgProcessingTimeMs = 0.0; ///< 平均处理时间
    };

    /** @brief 分析结果 */
    struct Result {
        double hurst = 0.0;            ///< Hurst指数
        double rSquared = 0.0;         ///< R^2拟合优度
        QVector<QPair<double, double>> rsPoints; ///< (log(n), log(R/S))点
        QString interpretation;        ///< 解释文本
    };

    explicit HurstExponent(QObject* parent = nullptr);

    /** @brief 计算Hurst指数(R/S分析) @param data 时间序列 @param minSegment 最小段长度 @return 分析结果 */
    Result compute(const QVector<double>& data, int minSegment = 8);

    /** @brief 滑动窗口Hurst分析 @param data 时间序列 @param windowSize 窗口大小 @param stepSize 步长 @return Hurst指数序列 */
    QVector<double> slidingWindow(const QVector<double>& data,
                                  int windowSize, int stepSize = 0);

    /** @brief R/S统计量 @param data 数据段 @return R/S值 */
    double rsStatistic(const QVector<double>& data);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 分析完成 @param hurst Hurst指数 @param interpretation 解释 */
    void analysisCompleted(double hurst, const QString& interpretation);

private:
    Stats m_stats;
    double m_timeSum;
};

#endif // HURSTEXPONENT_H
