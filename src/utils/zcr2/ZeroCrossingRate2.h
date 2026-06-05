/**
 * @file ZeroCrossingRate2.h
 * @brief 过零率V2 — 带阈值+分帧
 *
 * 功能: 改进的过零率计算，支持可调阈值(非精确零点过零)和分帧模式。
 *       通过阈值避免低幅度噪声导致的虚假过零，分帧模式可分析
 *       过零率随时间的变化，用于语音端点检测和频率估计。
 *
 * 协作: ShortTimeEnergy(短时能量) / PitchDetector2(音高检测)
 */
#ifndef ZEROCROSSINGRATE2_H
#define ZEROCROSSINGRATE2_H

#include <QObject>
#include <QVector>

/**
 * @brief 过零率V2 — 带阈值+分帧
 */
class ZeroCrossingRate2 : public QObject {
    Q_OBJECT

public:
    /** @brief 统计信息 */
    struct Stats {
        quint64 totalComputations = 0;     ///< 累计计算次数
        double  avgProcessingTimeMs = 0.0; ///< 平均处理时间(ms)
    };

    explicit ZeroCrossingRate2(QObject* parent = nullptr);

    /** @brief 计算分帧过零率
     *  @param signal 输入信号
     *  @param frameSize 帧长(样本数)
     *  @param threshold 过零阈值(幅度容差)
     *  @return 每帧过零率 */
    QVector<double> compute(const QVector<double>& signal,
                            int frameSize = 256,
                            double threshold = 0.0);

    /** @brief 基于过零率的语音段检测
     *  @param signal 输入信号
     *  @return 有声标志列表(true=有声) */
    QVector<bool> detectVoice(const QVector<double>& signal);

    /** @brief 计算全信号过零率
     *  @param signal 输入信号
     *  @param threshold 过零阈值
     *  @return 过零率(过零次数/总样本数) */
    double computeGlobal(const QVector<double>& signal,
                         double threshold = 0.0) const;

    /** @brief 设置语音检测过零率阈值 @param threshold 阈值 */
    void setVoiceThreshold(double threshold);

    /** @brief 获取统计 */
    const Stats& stats() const { return m_stats; }

    /** @brief 重置统计 */
    void resetStatistics();

signals:
    /** @brief 计算完成 @param numFrames 帧数 */
    void computationCompleted(int numFrames);

private:
    double m_voiceThreshold;  ///< 语音检测阈值
    double m_timeSum;         ///< 处理时间累加器
    Stats  m_stats;           ///< 统计信息
};

#endif // ZEROCROSSINGRATE2_H
