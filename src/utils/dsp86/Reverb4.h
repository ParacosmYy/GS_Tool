#pragma once
#include <QObject>
#include <QVector>

/**
 * @brief 混响音效处理器
 *
 * 模拟房间混响效果，支持Schroeder/FDN算法。
 */
class Reverb4 : public QObject {
    Q_OBJECT
public:
    struct Stats {
        int totalSamplesProcessed = 0;
        int totalBuffersApplied = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit Reverb4(QObject* parent = nullptr);

    /** @brief 处理音频缓冲区 */
    QVector<double> process(const QVector<double>& input);

    /** @brief 设置房间参数(大小/衰减/预延迟) */
    void setRoom(double roomSize, double damping, double preDelayMs);

    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    void effectApplied(int sampleCount, double roomSize);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
    double m_roomSize = 0.5;
    double m_damping = 0.5;
};
