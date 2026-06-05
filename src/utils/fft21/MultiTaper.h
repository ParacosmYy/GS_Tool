/**
 * @file MultiTaper.h
 * @brief 多锥谱估计引擎 — Slepian/DPSS序列/自适应加权/特征谱平均
 *
 * 功能: 实现多锥(Multitaper)谱估计方法，使用Slepian离散扁长椭球
 *       序列(DPSS)作为正交锥，支持自适应加权合并特征谱，
 *       通过带宽参数控制频率分辨率与方差之间的平衡。
 *
 * 协作: SpectrumAnalyzer(频谱分析) / WaveformGenerator(波形生成)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 多锥谱估计引擎 — DPSS锥与自适应加权谱估计
 */
class MultiTaper : public QObject {
    Q_OBJECT

public:
    /** @brief 谱估计结果 */
    struct SpectrumResult {
        QVector<double> frequencies;    ///< 频率轴(Hz)
        QVector<double> powerSpectrum;  ///< 功率谱密度
        QVector<double> amplitudeSpectrum; ///< 幅度谱
        double noiseFloor = 0.0;        ///< 噪声底
        double bandwidth = 0.0;         ///< 有效带宽(Hz)
    };

    /** @brief 统计 */
    struct Stats {
        quint64 totalEstimations = 0;            ///< 累计估计次数
        quint64 totalSamplesProcessed = 0;       ///< 累计处理采样数
        double  avgProcessingTimeMs = 0.0;       ///< 平均处理时间(ms)
        double  avgNoiseFloor = 0.0;            ///< 平均噪声底
        quint64 totalTapersUsed = 0;            ///< 累计使用锥数
    };

    explicit MultiTaper(QObject* parent = nullptr);

    /** @brief 设置采样率 @param rate 采样率(Hz) */
    void setSampleRate(double rate);

    /** @brief 设置带宽参数(NW) @param nw 时间带宽积(典型值2-4) */
    void setBandwidthParameter(double nw);

    /** @brief 设置锥数量 @param numTapers 锥数(≤2*NW) */
    void setNumTapers(int numTapers);

    /** @brief 计算多锥谱估计 @param data 时域输入数据 @return 谱估计结果 */
    SpectrumResult estimate(const QVector<double>& data);

    /** @brief 计算Slepian/DPSS序列 @param n 序列长度 @param nw 带宽参数 @param k 锥序号 @return DPSS序列 */
    QVector<double> computeDPSS(int n, double nw, int k) const;

    /** @brief 自适应加权合并特征谱 @param eigenspectra 特征谱列表 @param n 数据长度 @return 加权功率谱 */
    QVector<double> adaptiveWeightedAverage(
        const QVector<QVector<double>>& eigenspectra, int n) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 谱估计完成 @param sampleCount 采样数 @param numTapers 使用锥数 @param bandwidth 有效带宽 */
    void estimationComplete(int sampleCount, int numTapers, double bandwidth);

private:
    void fft(QVector<double>& real, QVector<double>& imag) const;
    QVector<double> tridiagonalSolve(const QVector<double>& diag,
                                     const QVector<double>& offDiag,
                                     int numEigen) const;
    double dpssConcentration(int n, int k, double nw,
                             const QVector<double>& v) const;

    double m_sampleRate;            ///< 采样率
    double m_nw;                    ///< 带宽参数(NW)
    int m_numTapers;               ///< 锥数量

    Stats m_stats;
    double m_timeSum = 0.0;         ///< 处理时间累加器
};
