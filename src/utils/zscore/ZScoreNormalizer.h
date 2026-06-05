/**
 * @file ZScoreNormalizer.h
 * @brief Z-Score标准化器 — 实时数据标准化
 *
 * 功能: 使用运行均值和方差实时Z-Score标准化，
 *       支持批量和流式模式，统计标准化次数/异常比例。
 */
#ifndef ZSCORENORMALIZER_H
#define ZSCORENORMALIZER_H

#include <QObject>
#include <QVector>

/**
 * @class ZScoreNormalizer
 * @brief 实时Z-Score标准化，支持流式更新
 */
class ZScoreNormalizer : public QObject {
    Q_OBJECT
public:
    /** 标准化统计 */
    struct Stats {
        quint64 totalNormalized = 0;
        quint64 totalOutliers = 0;      ///< |z| > 3的次数
        double  outlierRate = 0.0;
        double  currentMean = 0.0;
        double  currentStddev = 0.0;
        double  averageProcessingTimeMs = 0.0;
    };

    explicit ZScoreNormalizer(QObject* parent = nullptr);

    /** 设置异常阈值 */
    void setOutlierThreshold(double z);

    /** 批量标准化 */
    QVector<double> normalize(const QVector<double>& data);

    /** 流式: 更新统计并返回标准化值 */
    double normalizeSingle(double value);

    /** 反标准化 */
    double denormalize(double zScore) const;

    /** 查询 */
    double mean() const;
    double stddev() const;
    int count() const;

    void reset();
    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void outlierDetected(int index, double value, double zScore);
    void statsUpdated(double mean, double stddev);

private:
    void updateStats(double value);

    double m_outlierThreshold;
    double m_sum;
    double m_sqSum;
    int m_count;
    Stats m_stats;
    double m_timeSum;
};

#endif // ZSCORENORMALIZER_H
