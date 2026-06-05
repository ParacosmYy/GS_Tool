#pragma once
#include <QObject>
#include <QVector>

/**
 * @brief 包络检测器
 *
 * 通过可调攻击/释放时间的峰值检测或RMS平滑提取信号包络,
 * 适用于音频动态处理前置检测、调制分析与语音活动检测。
 */
class EnvelopeDetect7 : public QObject {
    Q_OBJECT
public:
    /// 统计信息结构体
    struct Stats { int totalDetected = 0; double avgProcessingTimeMs = 0.0; };

    explicit EnvelopeDetect7(QObject* parent = nullptr);

    /** @brief 设置攻击时间常数(秒) */
    void setAttackTime(double seconds);

    /** @brief 设置释放时间常数(秒) */
    void setReleaseTime(double seconds);

    /** @brief 对输入信号执行包络检测 */
    void detect(const QVector<double>& samples);

    /** @brief 获取当前统计信息 */
    Stats stats() const { return m_stats; }

    /** @brief 重置统计数据 */
    void resetStatistics();

signals:
    /** @brief 检测完成信号,返回包络数据 */
    void detected(const QVector<double>& envelope);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
    double m_attackTime = 0.01;
    double m_releaseTime = 0.1;
};
