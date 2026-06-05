/**
 * @file HarmonicProduct.h
 * @brief 谐波乘积谱 — 基频检测/自相关验证
 *
 * 功能: 通过谐波乘积谱(HPS)算法检测信号基频，支持降采样频谱
 *       乘积、自相关交叉验证、多候选基频排序。
 *
 * 协作: SpectrumAnalyzer(频谱计算) / PeakDetector(峰值检测)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QList>
#include <QPair>

/**
 * @brief 谐波乘积谱 — 基频(音高)检测引擎
 */
class HarmonicProduct : public QObject {
    Q_OBJECT

public:
    /** @brief 基频检测结果 */
    struct PitchResult {
        double frequency = 0.0;        ///< 检测到的基频(Hz)
        double confidence = 0.0;       ///< 置信度(0-1)
        double hpsMagnitude = 0.0;     ///< HPS幅度
        bool   valid = false;          ///< 检测是否有效
    };

    /** @brief 统计 */
    struct Stats {
        quint64 totalDetections = 0;           ///< 累计检测次数
        quint64 totalSpectraProcessed = 0;     ///< 累计处理频谱数
        quint64 totalValidPitches = 0;         ///< 累计有效基频数
        double  avgProcessingTimeMs = 0.0;     ///< 平均处理时间(ms)
        double  avgConfidence = 0.0;           ///< 平均检测置信度
    };

    explicit HarmonicProduct(QObject* parent = nullptr);

    /** @brief 设置谐波乘积阶数 @param order 阶数(>=2) */
    void setHarmonicOrder(int order);

    /** @brief 设置频率搜索范围 @param minHz 最低频率 @param maxHz 最高频率 */
    void setSearchRange(double minHz, double maxHz);

    /**
     * @brief 从频谱计算谐波乘积谱
     * @param magnitude 幅度谱
     * @param frequencies 频率轴
     * @return HPS结果(与输入等长)
     */
    QVector<double> computeHPS(const QVector<double>& magnitude,
                               const QVector<double>& frequencies);

    /**
     * @brief 检测基频
     * @param magnitude 幅度谱
     * @param frequencies 频率轴
     * @param sampleRate 采样率
     * @return 基频检测结果
     */
    PitchResult detectPitch(const QVector<double>& magnitude,
                            const QVector<double>& frequencies,
                            double sampleRate);

    /**
     * @brief 自相关验证
     * @param samples 时域采样
     * @param candidateFreq 候选基频
     * @param sampleRate 采样率
     * @return 自相关值(0-1)
     */
    double autocorrelationVerify(const QVector<double>& samples,
                                 double candidateFreq,
                                 double sampleRate);

    /**
     * @brief 降采样频谱(整数倍)
     * @param spectrum 输入频谱
     * @param factor 降采样因子
     * @return 降采样后的频谱
     */
    QVector<double> downsampleSpectrum(const QVector<double>& spectrum,
                                       int factor);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 基频检测完成 @param freq 基频 @param confidence 置信度 */
    void pitchDetected(double freq, double confidence);

    /** @brief HPS计算完成 @param order 阶数 @param peakBin 峰值bin */
    void hpsComputed(int order, int peakBin);

private:
    int findHpsPeak(const QVector<double>& hps, int minBin, int maxBin);

    int m_harmonicOrder;             ///< 谐波乘积阶数
    double m_minFreq;                ///< 搜索最低频率
    double m_maxFreq;                ///< 搜索最高频率

    Stats m_stats;
    double m_timeSum = 0.0;     ///< 处理时间累加器
};
