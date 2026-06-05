/**
 * @file EnvelopeDetect.h
 * @brief 包络检测引擎 — Hilbert包络/峰值插值/Attack-Release平滑/RMS包络
 *
 * 功能: 实现多种信号包络检测方法，包括基于Hilbert变换的瞬时包络、
 *       带抛物线插值的峰值检测、Attack/Release时间常数平滑、
 *       以及RMS滑动窗包络。适用于信号幅度分析和调制解调。
 *
 * 协作: PeakDetector(峰值检测) / DataSmoother(数据平滑)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 包络检测引擎 — Hilbert变换/峰值检测/RMS包络
 */
class EnvelopeDetect : public QObject {
    Q_OBJECT

public:
    /** @brief 包络类型 */
    enum class EnvelopeType {
        Hilbert,        ///< Hilbert变换瞬时包络
        Peak,           ///< 峰值检测包络
        RMS,            ///< RMS滑动窗包络
        Smoothed        ///< Attack/Release平滑包络
    };
    Q_ENUM(EnvelopeType)

    /** @brief 峰值信息 */
    struct PeakInfo {
        int index = 0;              ///< 峰值位置索引
        double interpolatedIndex = 0.0; ///< 插值后的精确位置
        double value = 0.0;         ///< 峰值幅度
        double interpolatedValue = 0.0; ///< 插值后的精确幅度
    };

    /** @brief 包络结果 */
    struct EnvelopeResult {
        QVector<double> envelope;       ///< 包络曲线
        QVector<PeakInfo> peaks;        ///< 检测到的峰值
        double peakAmplitude = 0.0;     ///< 最大峰值幅度
        double rmsLevel = 0.0;         ///< RMS电平
    };

    /** @brief 统计 */
    struct Stats {
        quint64 totalDetections = 0;            ///< 累计检测次数
        quint64 totalSamplesProcessed = 0;      ///< 累计处理采样数
        double  avgProcessingTimeMs = 0.0;      ///< 平均处理时间(ms)
        quint64 totalPeaksDetected = 0;         ///< 累计检测峰值数
        double  avgPeakAmplitude = 0.0;        ///< 平均峰值幅度
    };

    explicit EnvelopeDetect(QObject* parent = nullptr);

    /** @brief 设置包络类型 @param type 包络检测类型 */
    void setEnvelopeType(EnvelopeType type);

    /** @brief 设置Attack时间 @param ms Attack时间(ms) */
    void setAttackTime(double ms);

    /** @brief 设置Release时间 @param ms Release时间(ms) */
    void setReleaseTime(double ms);

    /** @brief 设置RMS窗口大小 @param size 窗口采样数 */
    void setRmsWindowSize(int size);

    /** @brief 设置采样率 @param rate 采样率(Hz) */
    void setSampleRate(double rate);

    /** @brief 检测信号包络 @param data 输入信号 @return 包络结果 */
    EnvelopeResult detect(const QVector<double>& data);

    /** @brief Hilbert变换计算瞬时包络 @param data 输入信号 @return 解析信号幅度 */
    QVector<double> hilbertEnvelope(const QVector<double>& data);

    /** @brief 带插值的峰值检测 @param data 输入信号 @return 峰值列表 */
    QVector<PeakInfo> detectPeaks(const QVector<double>& data) const;

    /** @brief Attack/Release平滑 @param data 输入信号 @return 平滑后的信号 */
    QVector<double> attackReleaseSmooth(const QVector<double>& data);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 检测完成 @param sampleCount 采样数 @param peakCount 峰值数 @param peakAmp 峰值幅度 */
    void detectionComplete(int sampleCount, int peakCount, double peakAmp);

private:
    void fft(QVector<double>& real, QVector<double>& imag) const;
    PeakInfo interpolatePeak(const QVector<double>& data, int idx) const;
    double computeRms(const QVector<double>& data, int start, int len) const;

    EnvelopeType m_type;            ///< 包络类型
    double m_attackTime;            ///< Attack时间(ms)
    double m_releaseTime;           ///< Release时间(ms)
    int m_rmsWindowSize;            ///< RMS窗口大小
    double m_sampleRate;            ///< 采样率

    /** @brief 平滑滤波器状态 */
    double m_envelopeState;         ///< 当前包络状态
    double m_attackCoeff;           ///< Attack系数
    double m_releaseCoeff;          ///< Release系数

    Stats m_stats;
    double m_timeSum = 0.0;         ///< 处理时间累加器
};
