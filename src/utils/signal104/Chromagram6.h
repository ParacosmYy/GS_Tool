#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 色度图(Chromagram)计算器
 *
 * 将音频频谱映射到12个色度音级(Pitch Class)，
 * 广泛用于音乐信息检索中的和弦识别和音调分析。
 */
class Chromagram6 : public QObject {
    Q_OBJECT
public:
    /// 统计信息结构体
    struct Stats { int totalComputed = 0; double avgProcessingTimeMs = 0.0; };

    explicit Chromagram6(QObject* parent = nullptr);

    /** @brief 设置八度范围 */
    void setOctaveRange(int minOctave, int maxOctave);

    /** @brief 设置每八度的频率箱数 */
    void setBins(int binsPerOctave);

    /** @brief 计算色度特征向量 */
    QVector<double> compute(const QVector<double>& samples);

    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 计算完成信号 */
    void computed(int frameCount);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
    int m_minOctave = 2;
    int m_maxOctave = 6;
    int m_bins = 12;
};
