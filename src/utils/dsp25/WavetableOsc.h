/**
 * @file WavetableOsc.h
 * @brief 波表合成器 — 多维波表+插值
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 波表合成器
 * 支持多维波表(frame×position), 多种插值, 抗混叠
 */
class WavetableOsc : public QObject
{
    Q_OBJECT

public:
    /** @brief 统计信息 */
    struct Stats {
        int totalSamplesGenerated = 0;
        int totalNotesPlayed = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit WavetableOsc(int tableSize = 2048, QObject* parent = nullptr);

    /** @brief 加载波表 @param wavetable 单帧波表 @param sampleRate 采样率 */
    void loadWavetable(const QVector<double>& wavetable, double sampleRate);

    /** @brief 加载多维波表 @param frames 波表帧列表(从柔和到明亮) @param sampleRate 采样率 */
    void loadMultidimWavetable(const QVector<QVector<double>>& frames,
                               double sampleRate);

    /** @brief 生成单个采样 @return 输出采样值[-1,1] */
    double tick();

    /** @brief 生成多个采样 @param count 采样数 @return 输出缓冲区 */
    QVector<double> generate(int count);

    /** @brief 设置频率 @param freq 频率(Hz) */
    void setFrequency(double freq);

    /** @brief 设置波表位置(多维) @param position [0,1] */
    void setPosition(double position);

    /** @brief 重置相位 */
    void reset();

    /** @brief 获取当前频率 */
    double frequency() const { return m_frequency; }
    /** @brief 获取波表大小 */
    int tableSize() const { return m_tableSize; }

    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    void frequencyChanged(double freq);

private:
    /** @brief 线性插值查表 */
    double interpolate(double phase, int frame) const;

    int m_tableSize;                     ///< 单帧波表大小
    double m_sampleRate = 44100.0;       ///< 采样率
    double m_frequency = 440.0;          ///< 当前频率
    double m_phase = 0.0;                ///< 当前相位[0,1)
    double m_phaseIncrement = 0.0;       ///< 相位增量
    double m_position = 0.0;             ///< 多维位置[0,1]
    QVector<QVector<double>> m_frames;   ///< 多帧波表

    Stats m_stats;
    double m_timeSum = 0.0;
};
