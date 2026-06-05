/**
 * @file SpectrumMonitor.h
 * @brief 频谱监控引擎 — FFT频谱计算/峰值保持/瀑布图累积
 *
 * 设计: QObject子类、接收采样数据→加窗FFT→dBFS幅度谱→瀑布图矩阵累积
 * 协作: SpectrumMonitorWidget(显示) / SpectrumTypes(数据结构)
 *
 * 算法:
 *   1. feedData()累积原始采样到环形缓冲区
 *   2. processBuffer()按overlap比例取帧→加窗→FFT→幅度谱(dBFS)
 *   3. 峰值保持: 逐频率bin取历史最大值，指数衰减
 *   4. 瀑布图: 每帧频谱追加到m_spectrogram(time x freq矩阵)
 */

#ifndef SPECTRUMMONITOR_H
#define SPECTRUMMONITOR_H

#include <QObject>
#include <QVector>
#include <QElapsedTimer>
#include <complex>

#include "chart/spectrum/SpectrumTypes.h"

/** @brief 频谱监控引擎 — 实时FFT频谱分析、峰值保持与瀑布图累积 */
class SpectrumMonitor : public QObject {
    Q_OBJECT

public:
    /** @brief 构造频谱监控引擎 @param parent 父对象 */
    explicit SpectrumMonitor(QObject* parent = nullptr);

    // ---- 参数配置 ----

    /** @brief 设置频谱分析参数(采样率/FFT大小/窗函数/重叠) @param params 参数集 */
    void setParams(const SpectrumParams& params);

    /** @brief 获取当前参数 @return 参数集 */
    SpectrumParams params() const;

    // ---- 数据输入 ----

    /** @brief 输入一批采样数据 @param samples 时域采样序列 */
    void feedData(const QVector<double>& samples);

    // ---- 数据输出 ----

    /** @brief 获取最新频谱切片 @return 频谱数据(freqs + magnitudes) */
    SpectrumSlice lastSlice() const;

    /** @brief 获取峰值保持频谱(dBFS) @return 峰值幅度向量 */
    QVector<double> peakHoldMagnitudes() const;

    /** @brief 获取瀑布图数据 @return 二维矩阵[time_row][freq_bin] */
    QVector<QVector<double>> spectrogram() const;

    /** @brief 获取瀑布图行数 @return 时间轴行数 */
    int spectrogramRows() const;

    /** @brief 获取频率轴(与频谱切片同长) @return 频率向量(Hz) */
    QVector<double> frequencyAxis() const;

    // ---- 控制 ----

    /** @brief 清除峰值保持数据 */
    void clearPeakHold();

    /** @brief 清除瀑布图数据 */
    void clearSpectrogram();

    /** @brief 清除所有数据(缓冲区+峰值+瀑布图) */
    void clearAll();

    // ---- 统计 ----

    /** @brief 获取累计处理的采样点数 */
    quint64 totalSamplesProcessed() const;

    /** @brief 获取累计执行的FFT次数 */
    quint64 totalFftExecuted() const;

    /** @brief 获取累计处理的帧数 */
    quint64 totalFramesProcessed() const;

    /** @brief 获取峰值频率(Hz) @return 最新频谱中的最大值频率 */
    double peakFrequency() const;

    /** @brief 获取峰值幅度(dBFS) @return 最新频谱中的最大值 */
    double peakMagnitude() const;

    /** @brief 重置所有统计计数器 */
    void resetStatistics();

signals:
    /** @brief 频谱更新信号(每次FFT完成后发射) @param slice 频谱切片 */
    void spectrumUpdated(const SpectrumSlice& slice);

    /** @brief 瀑布图更新信号(新数据行追加后发射) */
    void spectrogramUpdated();

private:
    /** @brief 处理缓冲区数据 — 按overlap取帧执行FFT */
    void processBuffer();

    /** @brief 执行单次FFT并生成频谱切片 @param frame 输入帧数据 @return 频谱切片 */
    SpectrumSlice computeSpectrum(const QVector<double>& frame);

    /** @brief 生成窗函数系数 @param size 窗口长度 @param wf 窗函数类型 */
    void generateWindow(int size, WindowFunction wf);

    /** @brief Cooley-Tukey radix-2 DIT FFT @param data 复数序列(原地) */
    void fftRadix2(QVector<std::complex<double>>& data);

    /** @brief 计算dBFS幅度谱 @param fftResult FFT输出 @return dBFS幅度向量 */
    QVector<double> computeMagnitudes(const QVector<std::complex<double>>& fftResult);

    /** @brief 更新峰值保持(指数衰减) @param magnitudes 新帧幅度 */
    void updatePeakHold(const QVector<double>& magnitudes);

    /** @brief 追加频谱到瀑布图 @param magnitudes dBFS幅度向量 */
    void appendSpectrogramRow(const QVector<double>& magnitudes);

    SpectrumParams m_params;                         ///< 分析参数
    QVector<double> m_sampleBuffer;                  ///< 采样缓冲区
    QVector<double> m_windowCoeffs;                  ///< 窗函数系数缓存
    SpectrumSlice   m_lastSlice;                     ///< 最新频谱切片
    QVector<double> m_peakHold;                      ///< 峰值保持幅度(dBFS)
    QVector<QVector<double>> m_spectrogram;          ///< 瀑布图矩阵[time][freq]
    int m_spectrogramMaxRows = 256;                  ///< 瀑布图最大行数
    QElapsedTimer m_elapsedTimer;                    ///< 时间戳计时器

    // 统计计数器
    quint64 m_totalSamplesProcessed  = 0;            ///< 累计采样点数
    quint64 m_totalFftExecuted       = 0;            ///< 累计FFT执行次数
    quint64 m_totalFramesProcessed   = 0;            ///< 累计帧数
    double  m_peakFrequency          = 0.0;          ///< 峰值频率(Hz)
    double  m_peakMagnitude          = -120.0;       ///< 峰值幅度(dBFS)
};

#endif // SPECTRUMMONITOR_H
