/**
 * @file ChangepointDetector.h
 * @brief 变点检测器 — 识别数据分布的突变点
 *
 * 功能: 支持CUSUM/PELT/滑动窗口/二元分割四种变点检测算法，
 *       统计检测次数/变点总数/平均检测耗时。
 */
#ifndef CHANGEPOINTDETECTOR_H
#define CHANGEPOINTDETECTOR_H

#include <QObject>
#include <QVector>
#include <QList>

/**
 * @class ChangepointDetector
 * @brief 检测数据序列中均值/方差/趋势发生突变的点
 */
class ChangepointDetector : public QObject {
    Q_OBJECT
public:
    /** 检测算法 */
    enum class Method {
        CUSUM,          ///< 累积和法
        PELT,           ///< 惩罚最优分割法
        SlidingWindow,  ///< 滑动窗口比较法
        BinarySegmentation ///< 二元分割法
    };

    /** 变点信息 */
    struct Changepoint {
        int index;          ///< 变点位置索引
        double confidence;  ///< 置信度 [0,1]
        double magnitude;   ///< 变化幅度(前后均值差)
    };

    /** 检测统计 */
    struct Stats {
        quint64 totalDetections = 0;        ///< 总检测次数
        quint64 totalChangepointsFound = 0; ///< 累计发现变点数
        double  averageProcessingTimeMs = 0.0;
        quint64 totalPointsProcessed = 0;
    };

    explicit ChangepointDetector(QObject* parent = nullptr);

    /** 设置参数 */
    void setMethod(Method m);
    void setPenalty(double p);
    void setMinSegmentLength(int len);
    void setWindowSize(int size);
    void setConfidenceThreshold(double t);

    /** 执行检测 */
    QList<Changepoint> detect(const QVector<double>& data);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void changepointFound(const Changepoint& cp);
    void detectionComplete(int changepointCount);

private:
    QList<Changepoint> cusum(const QVector<double>& data);
    QList<Changepoint> pelt(const QVector<double>& data);
    QList<Changepoint> slidingWindow(const QVector<double>& data);
    QList<Changepoint> binarySegmentation(const QVector<double>& data);
    double segmentCost(const QVector<double>& data, int start, int end) const;

    Method m_method;
    double m_penalty;
    int m_minSegmentLength;
    int m_windowSize;
    double m_confidenceThreshold;
    Stats m_stats;
    double m_timeSum;
};

#endif // CHANGEPOINTDETECTOR_H
