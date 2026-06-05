/**
 * @file TickEngine.h
 * @brief 精确节拍引擎 — 高精度定时回调调度
 *
 * 功能: 基于QElapsedTimer的精确节拍调度器，支持BPM设置、
 *       节拍回调、节拍统计、漂移检测，适用于音频/MIDI同步场景。
 */
#ifndef TICKENGINE_H
#define TICKENGINE_H

#include <QObject>
#include <QTimer>
#include <QElapsedTimer>

class TickEngine : public QObject {
    Q_OBJECT
public:
    /** 统计 */
    struct Stats {
        quint64 totalTicks = 0;
        quint64 totalStarts = 0;
        quint64 missedTicks = 0;
        double  avgJitterMs = 0.0;
        double  peakJitterMs = 0.0;
    };

    explicit TickEngine(QObject* parent = nullptr);
    ~TickEngine() override;

    /** @brief 设置BPM @param bpm 节拍/分钟(20~300) */
    void setBpm(double bpm);

    /** @brief 设置细分 @param subdivision 每拍细分数(1/2/4/8) */
    void setSubdivision(int subdivision);

    /** @brief 启动节拍 */
    void start();

    /** @brief 停止节拍 */
    void stop();

    /** @brief 是否运行中 */
    bool isRunning() const;

    double bpm() const { return m_bpm; }
    int subdivision() const { return m_subdivision; }
    double tickIntervalMs() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 节拍信号 @param tickNumber 当前节拍号 @param timestamp 精确时间戳(ms) */
    void tick(quint64 tickNumber, double timestamp);
    /** @brief 节拍漂移警告 @param driftMs 漂移量 */
    void driftDetected(double driftMs);

private:
    void onTimerTick();

    QTimer* m_timer;
    QElapsedTimer m_elapsed;
    double m_bpm;
    int m_subdivision;
    quint64 m_tickCount;
    double m_expectedNextMs;
    bool m_running;
    Stats m_stats;
    double m_jitterSum;
};

#endif // TICKENGINE_H
