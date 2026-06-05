#pragma once
#include <QObject>
#include <QVector>

/**
 * @brief 瞬态信号检测器
 *
 * 基于短时能量与频谱通量分析检测信号中的瞬态成分,
 * 适用于音频 onset 检测、冲击响应分析与异常脉冲捕捉。
 */
class TransientDetect3 : public QObject {
    Q_OBJECT
public:
    /// 统计信息结构体
    struct Stats { int totalDetected = 0; double avgProcessingTimeMs = 0.0; };

    explicit TransientDetect3(QObject* parent = nullptr);

    /** @brief 设置检测灵敏度 */
    void setSensitivity(double sensitivity);

    /** @brief 对输入信号执行瞬态检测 */
    void detect(const QVector<double>& samples);

    /** @brief 获取当前统计信息 */
    Stats stats() const { return m_stats; }

    /** @brief 重置统计数据 */
    void resetStatistics();

signals:
    /** @brief 检测到瞬态事件信号 */
    void detected(int position);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
    double m_sensitivity = 1.0;
};
