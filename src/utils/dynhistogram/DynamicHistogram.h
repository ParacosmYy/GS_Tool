/**
 * @file DynamicHistogram.h
 * @brief 动态直方图构建器 — 等宽/等频/自定义分箱+统计特征
 *
 * 功能: 支持等宽/等频/自定义分箱的直方图构建，
 *       提供均值/方差/偏度/峰度/百分位统计，
 *       统计构建次数/数据量/耗时。
 */
#ifndef DYNAMICHISTOGRAM_H
#define DYNAMICHISTOGRAM_H

#include <QObject>
#include <QVector>

class DynamicHistogram : public QObject {
    Q_OBJECT
public:
    /** 分箱策略 */
    enum class BinStrategy { EqualWidth, EqualFrequency, Custom };

    /** 统计 */
    struct Stats {
        quint64 totalBuilds = 0;
        quint64 totalDataPoints = 0;
        double  averageProcessingTimeMs = 0.0;
    };

    /** 直方图结果 */
    struct Histogram {
        QVector<double> binEdges;    ///< 分箱边界(nBins+1)
        QVector<int> counts;         ///< 每箱计数
        QVector<double> densities;   ///< 每箱密度
        double mean = 0.0;
        double variance = 0.0;
        double skewness = 0.0;
        double kurtosis = 0.0;
        int nBins = 0;
        int totalPoints = 0;
    };

    explicit DynamicHistogram(QObject* parent = nullptr);

    /** @brief 构建直方图 @param data 输入数据 @param nBins 分箱数 @param strategy 分箱策略 @return 直方图 */
    Histogram build(const QVector<double>& data, int nBins = 30,
                    BinStrategy strategy = BinStrategy::EqualWidth);

    /** @brief 计算百分位 @param data 输入数据 @param percentile 百分位(0~100) @return 百分位值 */
    double percentile(const QVector<double>& data, double p) const;

    /** @brief 自动Sturges分箱数 @param dataSize 数据量 @return 建议分箱数 */
    int sturgesBins(int dataSize) const;

    /** @brief 自动Freedman-Diaconis分箱数 @param data 数据 @return 建议分箱数 */
    int freedmanDiaconisBins(const QVector<double>& data) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void buildCompleted(int nBins, int totalPoints);

private:
    Stats m_stats;
    double m_timeSum;
};

#endif // DYNAMICHISTOGRAM_H
