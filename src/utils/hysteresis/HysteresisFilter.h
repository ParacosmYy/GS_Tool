/**
 * @file HysteresisFilter.h
 * @brief 滞后滤波器 — 消除信号抖动和状态频繁翻转
 *
 * 功能: 可配置上下阈值/持续时间，防止信号在阈值附近抖动，
 *       统计状态切换次数/过滤掉的抖动次数。
 */
#ifndef HYSTERESISFILTER_H
#define HYSTERESISFILTER_H

#include <QObject>

/**
 * @class HysteresisFilter
 * @brief 滞后滤波器，用于消除传感器/信号抖动
 */
class HysteresisFilter : public QObject {
    Q_OBJECT
public:
    /** 滤波器状态 */
    enum class State {
        Low,    ///< 低状态
        High,   ///< 高状态
        Unknown ///< 未确定
    };

    /** 滤波器统计 */
    struct Stats {
        quint64 totalUpdates = 0;
        quint64 totalStateChanges = 0;
        quint64 totalBouncesFiltered = 0;
        double  highDurationMs = 0.0;
        double  lowDurationMs = 0.0;
        double  averageProcessingTimeMs = 0.0;
    };

    explicit HysteresisFilter(QObject* parent = nullptr);

    /** 设置阈值 */
    void setUpperThreshold(double threshold);
    void setLowerThreshold(double threshold);
    void setDebounceTimeMs(double ms);

    /** 更新输入值 */
    State update(double value, double dtMs = 0.0);

    /** 查询状态 */
    State currentState() const;
    double currentValue() const;
    bool isHigh() const;
    bool isLow() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();
    void reset();

signals:
    void stateChanged(State newState, State oldState);
    void bounceFiltered(double value);

private:
    double m_upperThreshold;
    double m_lowerThreshold;
    double m_debounceTimeMs;
    double m_currentValue;
    State m_state;
    double m_debounceTimer;
    State m_pendingState;
    double m_timeSum;
    Stats m_stats;
};

#endif // HYSTERESISFILTER_H
