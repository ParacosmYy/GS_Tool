/**
 * @file TimeStretch.h
 * @brief 时间拉伸引擎 — 相位声码器+WSOLA波形相似+分析合成重叠
 *
 * 功能: 基于相位声码器和WSOLA的时间拉伸，支持不改变音高的变速播放，
 *       分析/合成窗重叠叠加(OLA)，相位连续性校正。
 *
 * 协作: WaveformGenerator(波形生成) / SignalDecomposer(信号分解)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 时间拉伸处理器
 *
 * 相位声码器通过STFT分析/修改/合成实现变速不变调，
 * WSOLA通过波形相似度匹配保证时域连续性。
 */
class TimeStretch : public QObject {
    Q_OBJECT

public:
    /** @brief 拉伸算法 */
    enum class Algorithm {
        PhaseVocoder,   ///< 相位声码器(STFT域)
        WSOLA           ///< 波形相似度重叠叠加(时域)
    };
    Q_ENUM(Algorithm)

    /** @brief 统计信息 */
    struct Stats {
        quint64 totalFramesProcessed = 0;   ///< 累计处理帧数
        quint64 totalSamplesInput = 0;      ///< 累计输入采样点数
        quint64 totalSamplesOutput = 0;     ///< 累计输出采样点数
        double  avgProcessingTimeMs = 0.0;  ///< 平均处理时间(ms)
    };

    explicit TimeStretch(QObject* parent = nullptr);

    void setAlgorithm(Algorithm algorithm);
    void setStretchFactor(double factor);
    void setWindowSize(int size);
    void setHopAnalyse(int hop);
    void setSampleRate(double rate);

    QVector<double> process(const QVector<double>& input);
    QVector<double> flush();

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void frameProcessed(int inputSamples, int outputSamples);

private:
    QVector<double> processPhaseVocoder(const QVector<double>& input);
    QVector<double> processWSOLA(const QVector<double>& input);
    QVector<double> hannWindow(int size) const;
    double findBestMatch(const QVector<double>& buffer,
                         int position, int searchRange) const;

    Algorithm m_algorithm;          ///< 拉伸算法
    double m_stretchFactor;         ///< 拉伸因子(>1变慢, <1变快)
    int m_windowSize;               ///< 分析窗大小
    int m_hopAnalyse;               ///< 分析跳跃
    double m_sampleRate;            ///< 采样率

    /* 相位声码器状态 */
    QVector<double> m_prevPhase;    ///< 上一帧相位
    QVector<double> m_phaseCumul;   ///< 累积相位
    QVector<double> m_pvBuffer;     ///< 输出重叠缓冲区
    int m_pvWritePos;               ///< 输出写位置

    /* WSOLA状态 */
    QVector<double> m_wsolaBuffer;  ///< WSOLA输入缓冲区
    int m_wsolaReadPos;             ///< WSOLA读位置
    QVector<double> m_outputBuffer; ///< WSOLA输出缓冲区
    int m_outputWritePos;           ///< 输出写位置

    Stats m_stats;
    double m_timeSum = 0.0;
};
