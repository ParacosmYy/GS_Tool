/**
 * @file Reverb.h
 * @brief 算法混响引擎 — Schroeder梳状+全通+反馈延迟网络+早晚期混合
 *
 * 功能: 实现算法混响，包含 Schroeder 梳状滤波器组、全通滤波器、
 *       反馈延迟网络(FDN)、早期反射、晚期混响场、干湿比混合、预延迟。
 *
 * 协作: SignalGenerator(信号生成) / WaveformGenerator(波形分析)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 算法混响处理器
 *
 * 基于 Schroeder 拓扑和 FDN 结构的混响引擎，可模拟房间、
 * 大厅、板式等混响类型，支持预延迟和干湿比控制。
 */
class Reverb : public QObject {
    Q_OBJECT

public:
    /** @brief 混响预设类型 */
    enum class ReverbType {
        Room,       ///< 房间混响(短衰减)
        Hall,       ///< 大厅混响(长衰减)
        Plate,      ///< 板式混响(密度高)
        Spring      ///< 弹簧混响(金属感)
    };
    Q_ENUM(ReverbType)

    /** @brief 统计信息 */
    struct Stats {
        quint64 totalSamplesProcessed = 0;  ///< 累计处理采样点数
        quint64 totalFramesRendered = 0;    ///< 累计渲染帧数
        double  avgProcessingTimeMs = 0.0;  ///< 平均处理时间(ms)
    };

    explicit Reverb(QObject* parent = nullptr);

    void setSampleRate(double rate);
    void setReverbType(ReverbType type);
    void setRoomSize(double size);
    void setDamping(double damping);
    void setWetLevel(double wet);
    void setDryLevel(double dry);
    void setPreDelay(double delayMs);

    QVector<double> process(const QVector<double>& input);
    double processSample(double input);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void frameProcessed(int samples, double peakLevel);

private:
    void initFilters();
    void initSchroeder();
    void initFDN();

    double processComb(double input, int filterIdx);
    double processAllpass(double input, int filterIdx);
    double processEarlyReflections(double input);
    double processLateField(double input);

    double m_sampleRate;            ///< 采样率
    double m_roomSize;             ///< 房间大小[0,1]
    double m_damping;              ///< 阻尼系数[0,1]
    double m_wetLevel;             ///< 湿信号电平[0,1]
    double m_dryLevel;             ///< 干信号电平[0,1]
    double m_preDelayMs;           ///< 预延迟(ms)
    ReverbType m_reverbType;       ///< 混响类型

    /* 梳状滤波器组(4个) */
    static constexpr int kNumCombs = 4;
    QVector<QVector<double>> m_combBuffers;   ///< 梳状滤波器延迟线
    QVector<int> m_combIndices;                ///< 梳状滤波器写指针
    QVector<double> m_combFeedback;            ///< 梳状滤波器反馈系数
    QVector<double> m_combDamping;             ///< 梳状滤波器阻尼
    QVector<double> m_combLastFilter;          ///< 梳状滤波器上一低通值

    /* 全通滤波器组(2个) */
    static constexpr int kNumAllpass = 2;
    QVector<QVector<double>> m_allpassBuffers; ///< 全通延迟线
    QVector<int> m_allpassIndices;              ///< 全通写指针

    /* FDN反馈延迟网络(4线) */
    static constexpr int kFDNLines = 4;
    QVector<QVector<double>> m_fdnBuffers;     ///< FDN延迟线
    QVector<int> m_fdnIndices;                  ///< FDN写指针
    double m_fdnFeedback;                       ///< FDN反馈增益

    /* 预延迟缓冲 */
    QVector<double> m_preDelayBuffer;           ///< 预延迟缓冲区
    int m_preDelayIndex;                         ///< 预延迟写指针

    double m_earlyLevel;                        ///< 早期反射电平
    double m_lateLevel;                         ///< 晚期混响电平

    Stats m_stats;
    double m_timeSum = 0.0;
};
