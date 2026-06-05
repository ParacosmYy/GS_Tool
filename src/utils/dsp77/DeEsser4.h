#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief DeEsser4 - 去齿音处理器
 *
 * 检测并抑制语音中的齿音(sibilance)成分，
 * 使用频段检测和增益控制实现自然去齿音效果。
 */
class DeEsser4 : public QObject {
    Q_OBJECT
public:
    struct Stats {
        int totalFramesProcessed = 0;
        int totalSibilanceEvents = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit DeEsser4(QObject* parent = nullptr);

    /** @brief 设置齿音检测频率范围(Hz) */
    void setFrequencyRange(double minHz, double maxHz);

    /** @brief 设置抑制阈值(dB) */
    void setThreshold(double thresholdDb);

    /** @brief 处理音频帧，返回去齿音后的输出 */
    QVector<double> process(const QVector<double>& input);

    /** @brief 获取当前齿音增益 reduction */
    double currentReduction() const;

    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    void sibilanceDetected(double frequencyHz, double reductionDb);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
    double m_minHz = 4000.0;
    double m_maxHz = 9000.0;
    double m_threshold = -20.0;
};
