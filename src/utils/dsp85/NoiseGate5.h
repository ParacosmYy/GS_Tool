#pragma once
#include <QObject>
#include <QVector>

/**
 * @brief 噪声门处理器
 *
 * 根据信号幅度自动开启/关闭音频通道，抑制背景噪声。
 */
class NoiseGate5 : public QObject {
    Q_OBJECT
public:
    struct Stats {
        int totalSamplesProcessed = 0;
        int totalGateEvents = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit NoiseGate5(QObject* parent = nullptr);

    /** @brief 处理音频缓冲区 */
    QVector<double> process(const QVector<double>& input);

    /** @brief 设置门限参数(阈值/攻击/释放/保持) */
    void setParameters(double thresholdDb, double attackMs, double releaseMs, double holdMs);

    /** @brief 检测输入信号的RMS电平(dB) */
    double computeRmsLevel(const QVector<double>& input) const;

    /** @brief 获取当前门状态 */
    bool isGateOpen() const;

    /** @brief 获取当前阈值 */
    double threshold() const;

    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    void gateStateChanged(bool isOpen, double levelDb);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
    double m_threshold = -40.0;
    bool m_gateOpen = false;
};
