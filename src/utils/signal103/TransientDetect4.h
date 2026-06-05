#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 瞬态信号检测器
 *
 * 通过滑动窗口能量分析和自适应阈值检测信号中的瞬态事件，
 * 适用于冲击、爆破等非平稳信号的快速捕获。
 */
class TransientDetect4 : public QObject {
    Q_OBJECT
public:
    /// 统计信息结构体
    struct Stats { int totalDetected = 0; double avgProcessingTimeMs = 0.0; };

    explicit TransientDetect4(QObject* parent = nullptr);

    /** @brief 设置检测灵敏度 */
    void setSensitivity(double sensitivity);

    /** @brief 设置分析窗口大小 */
    void setWindowSize(int size);

    /** @brief 对信号执行瞬态检测 */
    QVector<int> detect(const QVector<double>& signal);

    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 检测到瞬态事件信号 */
    void detected(int position);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
    double m_sensitivity = 1.0;
    int m_windowSize = 256;
};
