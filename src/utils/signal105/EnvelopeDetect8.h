#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 信号包络检测器
 *
 * 通过可调攻击/释放时间常数的峰值跟踪实现包络提取，
 * 用于AM解调、动态范围控制和音频电平监测。
 */
class EnvelopeDetect8 : public QObject {
    Q_OBJECT
public:
    /// 统计信息结构体
    struct Stats { int totalDetected = 0; double avgProcessingTimeMs = 0.0; };

    explicit EnvelopeDetect8(QObject* parent = nullptr);

    /** @brief 设置攻击时间常数(ms) */
    void setAttackTime(double ms);

    /** @brief 设置释放时间常数(ms) */
    void setReleaseTime(double ms);

    /** @brief 检测信号包络 */
    QVector<double> detect(const QVector<double>& signal);

    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 包络检测完成信号 */
    void detected(QVector<double> envelope);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
    double m_attackTime = 10.0;
    double m_releaseTime = 100.0;
};
