/**
 * @file HistogramBuilder.h
 * @brief 直方图构建器 — 数据分布统计与可视化准备
 *
 * 功能: 自动分箱/等宽/等频/自定义分箱，统计频次/频率/累积分布，
 *       支持多分布拟合度检验。
 *
 * 协作: ByteFrequencyAnalyzer(字节频率) / ChartWidget(直方图显示)
 */
#ifndef HISTOGRAMBUILDER_H
#define HISTOGRAMBUILDER_H

#include <QObject>
#include <QVector>
#include <QList>
#include <QPair>

/**
 * @brief 直方图构建器 — 数据分布统计
 */
class HistogramBuilder : public QObject {
    Q_OBJECT

public:
    /** @brief 分箱策略 */
    enum class BinningStrategy {
        EqualWidth,     ///< 等宽分箱
        EqualFrequency, ///< 等频分箱
        Sturges,        ///< Sturges公式自动确定bin数
        FreedmanDiaconis ///< Freedman-Diaconis规则
    };
    Q_ENUM(BinningStrategy)

    /** @brief 直方图bin */
    struct Bin {
        double lowerBound = 0.0;    ///< 下界
        double upperBound = 0.0;    ///< 上界
        int count = 0;              ///< 频次
        double frequency = 0.0;     ///< 频率
        double cumulativeFreq = 0.0;///< 累积频率
    };

    /** @brief 统计 */
    struct Stats {
        quint64 totalValuesBinned = 0;  ///< 累计分箱值数
        quint64 totalHistogramsBuilt = 0;///< 累计构建直方图数
        double  averageBinCount = 0.0;  ///< 平均bin数
        int     peakBinCount = 0;       ///< 峰值bin数
    };

    explicit HistogramBuilder(QObject* parent = nullptr);

    /** @brief 设置分箱策略 @param strategy 策略 */
    void setBinningStrategy(BinningStrategy strategy);

    /** @brief 设置bin数量 @param count bin数 */
    void setBinCount(int count);

    /** @brief 构建直方图 @param data 数据 @return bin列表 */
    QList<Bin> build(const QVector<double>& data);

    /** @brief 构建直方图(自定义边界) @param data 数据 @param edges bin边界 @return bin列表 */
    QList<Bin> buildCustom(const QVector<double>& data,
                           const QVector<double>& edges);

    /** @brief 获取最近一次结果 @return bin列表 */
    QList<Bin> lastResult() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 直方图构建完成 @param binCount bin数 */
    void histogramReady(int binCount);

private:
    int computeOptimalBins(const QVector<double>& sortedData) const;

    BinningStrategy m_strategy;     ///< 分箱策略
    int m_binCount;                 ///< bin数量
    QList<Bin> m_lastResult;        ///< 最近结果

    Stats m_stats;
    double m_binCountSum;           ///< bin数累加器
};

#endif // HISTOGRAMBUILDER_H
