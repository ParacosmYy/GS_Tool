/**
 * @file AnomalyDetector.h
 * @brief 异常模式检测引擎 — 基于统计模型的实时异常检测
 *
 * 功能: 支持3种检测模式(基线偏差/突变检测/周期性异常)，
 *       实时检测数据流中的异常模式，支持自适应基线。
 *
 * 协作: OutlierDetector(离群点) / EventTimeline(异常事件记录)
 */
#ifndef ANOMALYDETECTOR_H
#define ANOMALYDETECTOR_H

#include <QObject>
#include <QVector>
#include <QList>
#include <QMap>
#include <QPair>

/**
 * @brief 异常模式检测引擎 — 统计模型实时异常检测
 */
class AnomalyDetector : public QObject {
    Q_OBJECT

public:
    /** @brief 异常类型 */
    enum class AnomalyType {
        Baseline,       ///< 基线偏差: 值偏离历史基线
        LevelShift,     ///< 级别突变: 均值突然跳变
        TrendChange,    ///< 趋势变化: 斜率方向翻转
        Stall,          ///< 停滞: 值长时间不变
        Spike           ///< 尖峰: 瞬时突变
    };
    Q_ENUM(AnomalyType)

    /** @brief 异常事件 */
    struct AnomalyEvent {
        int index = 0;              ///< 数据索引
        double value = 0.0;         ///< 异常值
        AnomalyType type;           ///< 异常类型
        double confidence = 0.0;    ///< 置信度(0-1)
        double baseline = 0.0;      ///< 基线值
    };

    /** @brief 统计 */
    struct Stats {
        quint64 totalEventsDetected = 0;    ///< 累计检测异常数
        quint64 totalPointsProcessed = 0;   ///< 累计处理点数
        double  anomalyRate = 0.0;          ///< 异常率
        double  peakConfidence = 0.0;       ///< 峰值置信度
        QMap<int, quint64> eventsByType;    ///< 各类型计数
    };

    explicit AnomalyDetector(QObject* parent = nullptr);

    /** @brief 设置窗口大小 @param size 窗口点数 */
    void setWindowSize(int size);

    /** @brief 设置检测灵敏度(0-1) @param sensitivity 灵敏度 */
    void setSensitivity(double sensitivity);

    /** @brief 检测批量数据中的异常 @param data 数据 @return 异常事件列表 */
    QList<AnomalyEvent> detect(const QVector<double>& data);

    /** @brief 流式检测单个值 @param value 新值 @return 异常事件(无异常时confidence=0) */
    AnomalyEvent detectPoint(double value);

    /** @brief 重置自适应基线 */
    void resetBaseline();

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 异常检测到 @param event 异常事件 */
    void anomalyDetected(const AnomalyEvent& event);

private:
    void updateBaseline(double value);
    double computeDeviation(double value) const;

    int m_windowSize;               ///< 基线窗口大小
    double m_sensitivity;           ///< 检测灵敏度
    double m_baselineMean;          ///< 基线均值
    double m_baselineStddev;        ///< 基线标准差
    QVector<double> m_buffer;       ///< 滑动窗口缓冲区
    double m_prevValue;             ///< 上一个值
    double m_prevSlope;             ///< 上一个斜率
    int m_stallCount;               ///< 停滞计数器

    Stats m_stats;
};

#endif // ANOMALYDETECTOR_H
