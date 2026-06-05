#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief StereoWidener6 - 立体声展宽器第6代实现
 *
 * 通过中/侧（Mid/Side）处理增强立体声宽度，
 * 支持可调展宽量、低频保持及单声道兼容性检查。
 */
class StereoWidener6 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalProcessFrames = 0; double avgProcessingTimeMs = 0.0; };
    explicit StereoWidener6(QObject* parent = nullptr);
    Stats stats() const { return m_stats; }
    void resetStatistics();

    /**
     * @brief 处理立体声帧进行展宽
     * @param leftFrame 左声道采样帧
     * @param rightFrame 右声道采样帧
     * @return 展宽后的 (左声道, 右声道) 帧
     */
    QPair<QVector<double>, QVector<double>> processFrame(
        const QVector<double>& leftFrame, const QVector<double>& rightFrame);

    /**
     * @brief 设置展宽量
     * @param width 展宽量 [0, 2]，1.0为原始宽度
     */
    void setWidth(double width);

    /**
     * @brief 设置低频保持频率（低于此频率不展宽）
     * @param frequencyHz 截止频率 (Hz)
     * @param sampleRate 采样率 (Hz)
     */
    void setLowFrequencyHold(double frequencyHz, double sampleRate);

    /**
     * @brief 检查单声道兼容性
     * @param leftFrame 左声道
     * @param rightFrame 右声道
     * @return 兼容性评分 [0, 1]，1为完全兼容
     */
    double checkMonoCompatibility(const QVector<double>& leftFrame,
                                  const QVector<double>& rightFrame) const;

signals:
    void wideningCompleted(int frameCount);

private:
    Stats m_stats; double m_timeSum = 0.0;
};
