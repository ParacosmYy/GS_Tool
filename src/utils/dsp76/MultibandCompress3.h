#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief MultibandCompress3 - 多频段动态压缩器
 *
 * 将信号分为多个频段独立压缩，支持交叉馈送
 * 和链接操作，适用于母带处理和广播。
 */
class MultibandCompress3 : public QObject {
    Q_OBJECT
public:
    struct Stats {
        int totalFramesProcessed = 0;
        int totalBandActivations = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit MultibandCompress3(QObject* parent = nullptr);

    /** @brief 设置频段分界频率(Hz) */
    void setCrossoverFreqs(const QVector<double>& frequencies);

    /** @brief 设置指定频段的压缩参数 */
    void setBandParams(int band, double thresholdDb, double ratio, double attackMs, double releaseMs);

    /** @brief 处理音频帧 */
    QVector<double> process(const QVector<double>& input);

    /** @brief 获取各频段当前增益 reduction */
    QVector<double> bandGainReductions() const;

    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    void bandCompressed(int band, double reductionDb);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
    int m_bandCount = 4;
    QVector<double> m_gainReductions;
};
