#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief GateExpand4 - 门限扩展处理器
 *
 * 智能门限控制，在信号低于阈值时使用可控的
 * 保持和释放参数平滑过渡，避免突然截断。
 */
class GateExpand4 : public QObject {
    Q_OBJECT
public:
    struct Stats {
        int totalFramesProcessed = 0;
        int totalGateOpenings = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit GateExpand4(QObject* parent = nullptr);

    /** @brief 设置门限阈值(dB) */
    void setThreshold(double thresholdDb);

    /** @brief 设置攻击/保持/释放时间(ms) */
    void setTiming(double attackMs, double holdMs, double releaseMs);

    /** @brief 处理音频帧 */
    QVector<double> process(const QVector<double>& input);

    /** @brief 获取门限状态: true=开 */
    bool isOpen() const;

    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    void gateStateChanged(bool open);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
    double m_threshold = -40.0;
    double m_attackMs = 1.0;
    double m_holdMs = 50.0;
    double m_releaseMs = 100.0;
};
