/**
 * @file OutlierDetector.h
 * @brief 离群值检测器 — 多算法识别异常数据点
 *
 * 功能: 支持Z-Score/IQR/MAD/Grubbs/DBSCAN五种离群值检测算法，
 *       统计检测到的离群点数/离群率等指标。
 */
#ifndef OUTLIERDETECTOR_H
#define OUTLIERDETECTOR_H

#include <QObject>
#include <QVector>
#include <QList>

/**
 * @class OutlierDetector
 * @brief 使用多种统计算法检测数据集中的离群值
 */
class OutlierDetector : public QObject {
    Q_OBJECT
public:
    /** 检测算法 */
    enum class Method {
        ZScore,     ///< Z-Score阈值法
        IQR,        ///< 四分位距法
        MAD,        ///< 中位数绝对偏差法
        Grubbs,     ///< Grubbs检验法
        DBSCAN      ///< 基于密度的聚类检测
    };

    /** 离群点信息 */
    struct Outlier {
        int index;          ///< 数据索引
        double value;       ///< 原始值
        double score;       ///< 离群分数(算法相关)
    };

    /** 检测统计 */
    struct Stats {
        quint64 totalDetections = 0;        ///< 总检测次数
        quint64 totalOutliersFound = 0;     ///< 累计发现离群点
        double  outlierRate = 0.0;          ///< 离群率
        double  averageProcessingTimeMs = 0.0;
        quint64 totalPointsProcessed = 0;
    };

    explicit OutlierDetector(QObject* parent = nullptr);

    /** 配置参数 */
    void setMethod(Method m);
    void setThreshold(double t);
    void setEpsilon(double eps);
    void setMinSamples(int minPts);

    /** 执行检测 */
    QList<Outlier> detect(const QVector<double>& data);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void outlierFound(const Outlier& o);
    void detectionComplete(int outlierCount, int totalCount);

private:
    QList<Outlier> zScore(const QVector<double>& data);
    QList<Outlier> iqrMethod(const QVector<double>& data);
    QList<Outlier> madMethod(const QVector<double>& data);
    QList<Outlier> grubbsMethod(const QVector<double>& data);
    QList<Outlier> dbscanMethod(const QVector<double>& data);

    Method m_method;
    double m_threshold;
    double m_epsilon;
    int m_minSamples;
    Stats m_stats;
    double m_timeSum;
};

#endif // OUTLIERDETECTOR_H
