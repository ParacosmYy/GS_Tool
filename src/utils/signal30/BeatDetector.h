/**
 * @file BeatDetector.h
 * @brief 节拍检测 — onset函数/自相关/节奏直方图/Tempo估计
 *
 * 音乐/音频节拍检测器，处理流程:
 *   - Onset检测函数: 基于频谱通量检测音符起始点
 *   - 自相关分析: 从onset序列估计周期性
 *   - 节奏直方图: 累计IOI(Inter-Onset Interval)分布
 *   - Tempo估计: 综合自相关和直方图结果得到BPM
 * 统计检测到的onset数、Tempo估计耗时。
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QElapsedTimer>

/**
 * @brief 节拍检测器
 */
class BeatDetector : public QObject {
    Q_OBJECT
public:
    /** 统计信息 */
    struct Stats {
        quint64 totalOnsetsDetected = 0;   ///< 总检测到的onset数
        quint64 totalFramesProcessed = 0;  ///< 总处理帧数
        quint64 totalTempoEstimates = 0;   ///< 总Tempo估计次数
        double avgProcessingTimeMs = 0.0;  ///< 平均处理耗时(ms)
    };

    /** 检测结果 */
    struct BeatResult {
        QVector<double> onsetTimes;    ///< onset时间点(秒)
        QVector<double> onsetStrength; ///< 每个onset的强度
        double estimatedTempoBPM;      ///< 估计的BPM
        double tempoConfidence;        ///< Tempo估计置信度(0~1)
        QVector<double> tempoHistogram; ///< 节奏直方图(BPM bins)
    };

    /**
     * @brief 构造函数
     * @param sampleRate 采样率
     * @param hopSize FFT帧移(默认512)
     * @param fftSize FFT大小(默认2048)
     * @param parent 父对象
     */
    explicit BeatDetector(double sampleRate = 44100.0, int hopSize = 512,
                          int fftSize = 2048, QObject* parent = nullptr);

    /**
     * @brief 检测音频中的节拍
     * @param samples 单声道音频采样
     * @return 检测结果
     */
    BeatResult detect(const QVector<float>& samples);

    /**
     * @brief 仅计算onset检测函数
     * @param samples 输入采样
     * @return onset强度曲线(每hop一个值)
     */
    QVector<double> computeOnsetFunction(const QVector<float>& samples);

    /**
     * @brief 从onset函数估计Tempo
     * @param onsetFunction onset强度曲线
     * @return BPM值
     */
    double estimateTempo(const QVector<double>& onsetFunction);

    /** @brief 获取统计 */
    const Stats& stats() const { return m_stats; }
    /** @brief 重置统计 */
    void resetStatistics();

    /** @brief 设置onset检测阈值(0.0~1.0) */
    void setOnsetThreshold(double threshold) { m_onsetThreshold = threshold; }
    /** @brief 设置BPM搜索范围 */
    void setTempoRange(double minBPM, double maxBPM);

signals:
    /** 检测完成 */
    void detectionComplete(int onsetCount, double tempoBPM, double processingTimeMs);
    /** onset检测完成 */
    void onsetDetected(double timeSec, double strength);

private:
    /** 计算幅度谱 */
    void computeMagnitudeSpectrum(const QVector<float>& frame,
                                  QVector<double>& mag) const;
    /** 频谱通量(半波整流) */
    double spectralFlux(const QVector<double>& prev, const QVector<double>& curr) const;
    /** onset峰值拾取 */
    QVector<int> pickPeaks(const QVector<double>& func, double threshold) const;
    /** 自相关 */
    QVector<double> autocorrelation(const QVector<double>& signal, int maxLag) const;
    /** 节奏直方图 */
    QVector<double> buildTempoHistogram(const QVector<double>& onsetFunc) const;

    double m_sampleRate;
    int m_hopSize;
    int m_fftSize;
    double m_onsetThreshold;
    double m_minBPM;
    double m_maxBPM;
    Stats m_stats;
    double m_timeSum = 0.0;
    QElapsedTimer m_timing;
};
