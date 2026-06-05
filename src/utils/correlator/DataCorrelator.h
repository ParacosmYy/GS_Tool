/**
 * @file DataCorrelator.h
 * @brief 数据相关性引擎 — 检测数据流之间的线性关系
 *
 * 功能: Pearson相关系数/自相关/互相关/滑动窗口实时相关，
 *       显著性检验，用于多通道数据分析。
 *
 * 协作: DataSynchronizer(对齐后相关性分析) / ChartWidget(相关矩阵显示)
 */
#ifndef DATACORRELATOR_H
#define DATACORRELATOR_H

#include <QObject>
#include <QVector>
#include <QList>
#include <QPair>
#include <QMap>

/**
 * @brief 数据相关性引擎 — 线性关系检测
 */
class DataCorrelator : public QObject {
    Q_OBJECT

public:
    /** @brief 统计 */
    struct Stats {
        quint64 totalCorrelationsComputed = 0;///< 累计计算次数
        double  averageCorrelation = 0.0;     ///< 平均相关系数
        double  peakCorrelation = 0.0;        ///< 峰值相关系数
        quint64 significantPairsFound = 0;    ///< 显著相关对数
    };

    explicit DataCorrelator(QObject* parent = nullptr);

    /** @brief 设置显著性阈值(默认0.05) @param threshold p-value阈值 */
    void setSignificanceThreshold(double threshold);

    /** @brief 计算Pearson相关系数 @param x 第一组数据 @param y 第二组数据 @return 相关系数(-1到1) */
    double pearsonCorrelation(const QVector<double>& x,
                              const QVector<double>& y);

    /** @brief 计算自相关 @param data 数据 @param lag 滞后阶数 @return 自相关系数 */
    double autoCorrelation(const QVector<double>& data, int lag);

    /** @brief 互相关(搜索最大相关的时间偏移) @param x 第一组数据 @param y 第二组数据 @param maxLag 最大搜索滞后 @return (最大相关系数, 最优滞后) */
    QPair<double, int> crossCorrelation(const QVector<double>& x,
                                        const QVector<double>& y,
                                        int maxLag);

    /** @brief 滑动窗口相关(实时) @param x 窗口X @param y 窗口Y @return 窗口相关系数 */
    double windowCorrelation(const QVector<double>& x,
                             const QVector<double>& y);

    /** @brief 近似p-value检验 @param r 相关系数 @param n 样本数 @return 近似p-value */
    static double approximatePValue(double r, int n);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 显著相关发现 @param r 相关系数 @param lag 滞后 @param pValue p值 */
    void significantCorrelation(double r, int lag, double pValue);

private:
    double m_significanceThreshold;     ///< 显著性阈值
    Stats m_stats;
    double m_corrSum;                   ///< 相关系数累加器
};

#endif // DATACORRELATOR_H
