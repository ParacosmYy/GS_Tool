#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 包络检测器实现
 *
 * 提取信号的瞬时幅值包络，支持Hilbert变换和峰值检波两种模式，
 * 适用于AM解调、振动信号特征提取和音频响度分析。
 */
class EnvelopeDetect9 : public QObject {
    Q_OBJECT
public:
    /// 统计信息结构体
    struct Stats { int totalDetected = 0; double avgProcessingTimeMs = 0.0; };

    explicit EnvelopeDetect9(QObject* parent = nullptr);

    /** @brief 设置检测模式：true=Hilbert变换，false=峰值检波 */
    void setHilbertMode(bool hilbert);

    /** @brief 设置后处理低通滤波器截止频率(Hz)，平滑包络曲线 */
    void setSmoothingCutoff(double freqHz);

    /** @brief 对输入信号执行包络检测，返回幅值包络 */
    QVector<double> detect(const QVector<double>& signal);

    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 检测完成信号，返回输出样本数 */
    void detectionCompleted(int sampleCount);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
    bool m_hilbertMode = true;
    double m_smoothingCutoff = 50.0;
};
