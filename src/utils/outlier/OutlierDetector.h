/**
 * @file OutlierDetector.h
 * @brief 异常值检测引擎 — 识别数据流中的离群点
 *
 * 功能: 支持4种检测方法(Z-Score/IQR/MAD/滚动窗口)，实时
 *       检测异常值，统计异常率/峰值偏移/分布特征。
 *
 * 协作: DataThresholdMonitor(告警联动) / DataQualityScorer(质量评估)
 */
#ifndef OUTLIERDETECTOR_H
#define OUTLIERDETECTOR_H

#include <QObject>
#include <QVector>
#include <QList>

/**
 * @brief 异常值检测引擎 — 识别数据流中的离群点
 */
class OutlierDetector : public QObject {
    Q_OBJECT

public:
    /** @brief 检测方法 */
    enum class DetectionMethod {
        ZScore,     ///< Z-Score: 超过N个标准差
        IQR,        ///< 四分位距: 低于Q1-1.5*IQR或高于Q3+1.5*IQR
        MAD,        ///< 中位数绝对偏差: 超过N*MAD
        RollingWindow ///< 滚动窗口: 与窗口均值偏差超过阈值
    };
    Q_ENUM(DetectionMethod)

    /** @brief 异常点 */
    struct Outlier {
        int index = 0;          ///< 数据索引
        double value = 0.0;     ///< 异常值
        double score = 0.0;     ///< 异常分数(越高越异常)
        double expected = 0.0;  ///< 预期值(均值/中位数)
    };

    /** @brief 统计 */
    struct Stats {
        quint64 totalPointsChecked = 0;    ///< 累计检查点数
        quint64 totalOutliersFound = 0;    ///< 累计发现异常数
        double  outlierRate = 0.0;         ///< 异常率(outliers/total)
        double  peakDeviation = 0.0;       ///< 峰值偏差
        double  averageScore = 0.0;        ///< 平均异常分数
    };

    explicit OutlierDetector(QObject* parent = nullptr);

    /** @brief 设置检测方法 @param method 方法 */
    void setMethod(DetectionMethod method);

    /** @brief 设置Z-Score阈值(默认3.0) @param threshold N倍标准差 */
    void setZScoreThreshold(double threshold);

    /** @brief 设置IQR倍数(默认1.5) @param multiplier 倍数 */
    void setIqrMultiplier(double multiplier);

    /** @brief 设置MAD阈值(默认3.0) @param threshold N倍MAD */
    void setMadThreshold(double threshold);

    /** @brief 设置滚动窗口大小 @param size 窗口点数 */
    void setWindowSize(int size);

    /** @brief 设置滚动窗口偏差阈值 @param threshold 阈值 */
    void setRollingThreshold(double threshold);

    /** @brief 检测数据中的异常值 @param data 数据 @return 异常点列表 */
    QList<Outlier> detect(const QVector<double>& data);

    /** @brief 实时检测单个新值 @param value 新值 @return 异常点(非异常时score=0) */
    Outlier detectPoint(double value);

    /** @brief 获取最近一次检测结果 @return 异常点列表 */
    QList<Outlier> lastOutliers() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 检测到异常值 @param outlier 异常点 */
    void outlierDetected(const Outlier& outlier);

    /** @brief 批量检测完成 @param outliers 异常点列表 */
    void detectionComplete(const QList<Outlier>& outliers);

private:
    QList<Outlier> detectZScore(const QVector<double>& data);
    QList<Outlier> detectIQR(const QVector<double>& data);
    QList<Outlier> detectMAD(const QVector<double>& data);
    QList<Outlier> detectRolling(const QVector<double>& data);

    DetectionMethod m_method;       ///< 检测方法
    double m_zThreshold;            ///< Z-Score阈值
    double m_iqrMultiplier;         ///< IQR倍数
    double m_madThreshold;          ///< MAD阈值
    int m_windowSize;               ///< 滚动窗口大小
    double m_rollingThreshold;      ///< 滚动窗口阈值
    QVector<double> m_streamBuffer; ///< 流式检测缓冲区
    QList<Outlier> m_lastOutliers;  ///< 最近检测结果

    Stats m_stats;
    double m_scoreSum;              ///< 异常分数累加器
};

#endif // OUTLIERDETECTOR_H
