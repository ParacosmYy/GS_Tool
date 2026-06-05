/**
 * @file PitchDetector2.h
 * @brief 音高检测V2 — 自相关+YIN+AMDF融合
 *
 * 功能: 改进的基频检测器，融合自相关、YIN和AMDF三种算法的结果，
 *       通过置信度加权投票获得更可靠的音高估计。支持单算法
 *       检测和融合检测两种模式。
 *
 * 协作: PitchDetector(基础检测) / ShortTimeEnergy(端点检测)
 */
#ifndef PITCHDETECTOR2_H
#define PITCHDETECTOR2_H

#include <QObject>
#include <QVector>

/**
 * @brief 音高检测V2 — 自相关+YIN+AMDF融合
 */
class PitchDetector2 : public QObject {
    Q_OBJECT

public:
    /** @brief 检测方法 */
    enum class Method {
        Autocorrelation,   ///< 自相关法
        Yin,               ///< YIN算法
        Amdf,              ///< AMDF算法
        Fused              ///< 三算法融合(默认)
    };
    Q_ENUM(Method)

    /** @brief 检测结果 */
    struct Result {
        double frequency = 0.0;     ///< 检测到的基频(Hz)
        double confidence = 0.0;    ///< 置信度[0,1]
        bool voiced = false;        ///< 是否有声段
    };

    /** @brief 统计信息 */
    struct Stats {
        quint64 totalDetections = 0;       ///< 累计检测次数
        double  avgProcessingTimeMs = 0.0; ///< 平均处理时间(ms)
    };

    explicit PitchDetector2(QObject* parent = nullptr);

    /** @brief 检测基频(默认融合模式)
     *  @param signal 输入信号帧
     *  @param sampleRate 采样率(Hz)
     *  @return 检测结果 */
    Result detect(const QVector<double>& signal, double sampleRate);

    /** @brief 使用指定方法检测
     *  @param signal 输入信号帧
     *  @param sampleRate 采样率(Hz)
     *  @param method 检测方法
     *  @return 检测结果 */
    Result detectReliable(const QVector<double>& signal,
                          double sampleRate, Method method);

    /** @brief 获取统计 */
    const Stats& stats() const { return m_stats; }

    /** @brief 重置统计 */
    void resetStatistics();

signals:
    /** @brief 检测完成 @param frequency 基频 @param confidence 置信度 */
    void detectionCompleted(double frequency, double confidence);

private:
    /** @brief 自相关检测 @param data 信号 @param sampleRate 采样率 @return 结果 */
    Result detectAutocorrelation(const QVector<double>& data, double sampleRate) const;

    /** @brief YIN检测 @param data 信号 @param sampleRate 采样率 @return 结果 */
    Result detectYin(const QVector<double>& data, double sampleRate) const;

    /** @brief AMDF检测 @param data 信号 @param sampleRate 采样率 @return 结果 */
    Result detectAmdf(const QVector<double>& data, double sampleRate) const;

    double m_timeSum;   ///< 处理时间累加器
    Stats  m_stats;     ///< 统计信息
};

#endif // PITCHDETECTOR2_H
