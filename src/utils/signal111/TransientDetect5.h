#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 瞬态信号检测器实现
 *
 * 基于短时能量和多分辨率分析检测信号中的瞬态事件(冲击、突发、阶跃)，
 * 适用于机械故障诊断、电力系统暂态分析和音频onset检测。
 */
class TransientDetect5 : public QObject {
    Q_OBJECT
public:
    /// 统计信息结构体
    struct Stats { int totalDetected = 0; double avgProcessingTimeMs = 0.0; };

    explicit TransientDetect5(QObject* parent = nullptr);

    /** @brief 设置检测灵敏度阈值(0.0~1.0)，值越高检测越严格 */
    void setSensitivity(double sensitivity);

    /** @brief 设置分析窗口长度(样本数)和滑动步长 */
    void setWindowParams(int windowSize, int hopSize);

    /** @brief 对输入信号执行瞬态检测，返回瞬态事件的起始位置和强度 */
    QVector<QPair<int, double>> detect(const QVector<double>& signal);

    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 检测完成信号，返回检测到的瞬态事件数 */
    void detectionCompleted(int eventCount);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
    double m_sensitivity = 0.7;
    int m_windowSize = 256;
    int m_hopSize = 64;
};
