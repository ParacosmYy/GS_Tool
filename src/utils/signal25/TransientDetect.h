/**
 * @file TransientDetect.h
 * @brief 瞬态检测引擎 — 起始检测/频谱通量/高频能量/自适应阈值
 *
 * 功能: 实现多种起始检测函数(频谱通量、高频能量、复数域)，
 *       自适应阈值峰值拾取，用于音频/信号瞬态事件检测。
 *
 * 协作: SpectrumAnalyzer(频谱分析) / PeakDetector(峰值检测)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QList>
#include <QPair>

/**
 * @brief 瞬态检测引擎
 */
class TransientDetect : public QObject {
    Q_OBJECT

public:
    /** @brief 检测函数类型 */
    enum class DetectFunction {
        SpectralFlux,     ///< 频谱通量
        HighFrequency,    ///< 高频能量
        ComplexDomain,    ///< 复数域检测
        PhaseDeviation    ///< 相位偏差
    };
    Q_ENUM(DetectFunction)

    /** @brief 检测到的瞬态事件 */
    struct TransientEvent {
        int frameIndex = 0;        ///< 帧索引
        double timePosition = 0.0; ///< 时间位置(秒)
        double strength = 0.0;     ///< 瞬态强度
        double confidence = 0.0;   ///< 置信度(0~1)
    };

    /** @brief 统计 */
    struct Stats {
        quint64 totalDetections = 0;          ///< 累计检测次数
        quint64 totalTransientsFound = 0;     ///< 累计发现瞬态数
        quint64 totalFramesProcessed = 0;     ///< 累计处理帧数
        double  avgProcessingTimeMs = 0.0;    ///< 平均处理时间(ms)
    };

    explicit TransientDetect(QObject* parent = nullptr);

    /** @brief 设置检测函数 @param func 检测函数 */
    void setDetectFunction(DetectFunction func);

    /** @brief 设置帧大小 @param size 帧大小(采样数) */
    void setFrameSize(int size);

    /** @brief 设置hop大小 @param size hop大小(采样数) */
    void setHopSize(int size);

    /** @brief 设置采样率 @param rate 采样率(Hz) */
    void setSampleRate(double rate);

    /** @brief 设置自适应阈值参数 @param multiplier 阈值乘数 @param windowSize 中值窗口 */
    void setThresholdParams(double multiplier, int windowSize);

    /** @brief 检测瞬态事件 @param data 时域数据 @return 瞬态事件列表 */
    QList<TransientEvent> detect(const QVector<double>& data);

    /** @brief 计算检测函数曲线 @param data 时域数据 @return (帧索引, 检测值) */
    QVector<QPair<int, double>> detectionFunction(const QVector<double>& data);

    /** @brief 自适应阈值峰值拾取 @param detectFunc 检测函数值 @return 峰值帧索引列表 */
    QList<int> adaptivePeakPick(const QVector<double>& detectFunc);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 检测完成 @param numTransients 瞬态数 @param numFrames 总帧数 */
    void detectionComplete(int numTransients, int numFrames);

    /** @brief 瞬态事件发现 @param frameIndex 帧索引 @param strength 强度 */
    void transientFound(int frameIndex, double strength);

private:
    QVector<double> computeSpectralFlux(const QVector<double>& prevFrame,
                                        const QVector<double>& curFrame);
    QVector<double> computeHighFreqContent(const QVector<double>& frame);
    double computeComplexDeviation(const QVector<double>& prevMag,
                                   const QVector<double>& curMag,
                                   const QVector<double>& prevPhase,
                                   const QVector<double>& curPhase);
    QVector<double> applyWindow(const QVector<double>& data) const;
    void forwardFFT(QVector<double>& real, QVector<double>& imag);

    DetectFunction m_func;          ///< 检测函数
    int m_frameSize;                ///< 帧大小
    int m_hopSize;                  ///< hop大小
    double m_sampleRate;            ///< 采样率
    double m_thresholdMultiplier;   ///< 阈值乘数
    int m_medianWindow;             ///< 中值窗口大小

    Stats m_stats;
    double m_timeSum = 0.0;         ///< 处理时间累加器
};
