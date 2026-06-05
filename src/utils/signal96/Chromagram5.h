#pragma once
#include <QObject>
#include <QVector>

/**
 * @brief 色度图(Chromagram)计算器
 *
 * 将音频频谱映射到12个音级(Pitch Class)上,提取和声与调性特征,
 * 适用于和弦识别、音乐分析与音高匹配。
 */
class Chromagram5 : public QObject {
    Q_OBJECT
public:
    /// 统计信息结构体
    struct Stats { int totalComputed = 0; double avgProcessingTimeMs = 0.0; };

    explicit Chromagram5(QObject* parent = nullptr);

    /** @brief 设置八度范围 */
    void setOctaveRange(int minOctave, int maxOctave);

    /** @brief 计算输入信号的色度特征 */
    void compute(const QVector<double>& samples);

    /** @brief 获取当前统计信息 */
    Stats stats() const { return m_stats; }

    /** @brief 重置统计数据 */
    void resetStatistics();

signals:
    /** @brief 计算完成信号,返回帧数 */
    void computed(int frameCount);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
    int m_minOctave = 2;
    int m_maxOctave = 6;
};
