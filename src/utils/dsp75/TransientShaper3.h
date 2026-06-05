#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief TransientShaper3 - 瞬态塑形器
 *
 * 独立控制音频信号的瞬态和持续成分，
 * 通过包络检测和增益调制增强或衰减瞬态。
 */
class TransientShaper3 : public QObject {
    Q_OBJECT
public:
    struct Stats {
        int totalFramesProcessed = 0;
        int totalTransientEvents = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit TransientShaper3(QObject* parent = nullptr);

    /** @brief 设置瞬态增强/衰减量(dB) */
    void setTransientAmount(double amountDb);

    /** @brief 设置攻击时间(ms) */
    void setAttackTime(double attackMs);

    /** @brief 设置释放时间(ms) */
    void setReleaseTime(double releaseMs);

    /** @brief 处理音频帧 */
    QVector<double> process(const QVector<double>& input);

    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    void transientDetected(double amplitude);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
    double m_amountDb = 0.0;
    double m_attackMs = 1.0;
    double m_releaseMs = 50.0;
};
