/**
 * @file ShortTimeEnergy.h
 * @brief 短时能量 — 语音端点检测
 *
 * 功能: 基于分帧计算的短时能量分析，用于语音/音频信号的端点检测。
 *       支持可调帧长和帧移，提供基于能量阈值的语音段检测。
 *       适用于语音激活检测(VAD)、有声/无声段分割等。
 *
 * 协作: ZeroCrossingRate2(过零率) / PitchDetector2(音高检测)
 */
#ifndef SHORTTIMEENERGY_H
#define SHORTTIMEENERGY_H

#include <QObject>
#include <QVector>

/**
 * @brief 短时能量 — 语音端点检测
 */
class ShortTimeEnergy : public QObject {
    Q_OBJECT

public:
    /** @brief 统计信息 */
    struct Stats {
        quint64 totalComputations = 0;     ///< 累计计算次数
        double  avgProcessingTimeMs = 0.0; ///< 平均处理时间(ms)
    };

    /** @brief 语音段 */
    struct VoiceSegment {
        int startFrame = 0;  ///< 起始帧索引
        int endFrame = 0;    ///< 结束帧索引
        double energy = 0.0; ///< 该段平均能量
    };

    explicit ShortTimeEnergy(QObject* parent = nullptr);

    /** @brief 计算短时能量
     *  @param signal 输入信号
     *  @param frameSize 帧长(样本数)
     *  @param hopSize 帧移(样本数)
     *  @return 每帧能量值 */
    QVector<double> compute(const QVector<double>& signal,
                            int frameSize = 256,
                            int hopSize = 128);

    /** @brief 基于能量阈值的语音段检测
     *  @param signal 输入信号
     *  @param threshold 能量阈值(0~1, 相对最大能量)
     *  @return 语音段列表 */
    QVector<VoiceSegment> detectVoice(const QVector<double>& signal,
                                      double threshold = 0.1);

    /** @brief 获取统计 */
    const Stats& stats() const { return m_stats; }

    /** @brief 重置统计 */
    void resetStatistics();

signals:
    /** @brief 计算完成 @param numFrames 帧数 */
    void computationCompleted(int numFrames);

    /** @brief 语音检测完成 @param numSegments 检测到的语音段数 */
    void voiceDetected(int numSegments);

private:
    double m_timeSum;   ///< 处理时间累加器
    Stats  m_stats;     ///< 统计信息
};

#endif // SHORTTIMEENERGY_H
