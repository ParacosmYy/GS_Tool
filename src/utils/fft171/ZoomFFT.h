/**
 * @file ZoomFFT.h
 * @brief 缩放FFT(Chirp-z变换窄带高分辨率频谱分析) — Zoom FFT via Chirp-z Transform for Narrow-Band High-Resolution Spectral Analysis
 *
 * 功能: 实现Chirp-z变换缩放FFT，支持指定频率范围的高分辨率分析、
 *       复数FFT加速和频带选择，适用于嵌入式窄带信号分析。
 *
 * 协作: SlidingDFT4(滑动DFT) / FftEngine(FFT) / GoertzelFilter(单频检测)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 缩放FFT处理器
 */
class ZoomFFT : public QObject {
    Q_OBJECT

public:
    /** @brief 运行统计 */
    struct Stats {
        quint64 totalTransforms = 0;      ///< 累计变换次数
        double avgProcessingTimeMs = 0.0; ///< 平均耗时(ms)
        int lastFFTSize = 0;              ///< 最近FFT点数
        int lastZoomBins = 0;             ///< 最近缩放bin数
    };

    explicit ZoomFFT(QObject *parent = nullptr);
    ~ZoomFFT() override;

    /**
     * @brief 执行Chirp-z变换
     * @param input 输入信号
     * @param startBin 起始频率bin
     * @param endBin 结束频率bin
     * @param numBins 输出bin数
     * @return 缩放后的复数频谱(实部/虚部交替)
     */
    QVector<double> chirpZTransform(const QVector<double>& input,
                                     double startBin, double endBin, int numBins);

    /**
     * @brief 便捷缩放FFT
     * @param input 输入信号
     * @param centerFreqHz 中心频率(Hz)
     * @param bandwidthHz 带宽(Hz)
     * @param sampleRateHz 采样率(Hz)
     * @param numBins 输出bin数
     * @return 幅度谱
     */
    QVector<double> zoom(const QVector<double>& input,
                          double centerFreqHz, double bandwidthHz,
                          double sampleRateHz, int numBins);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 变换完成 @param bins 输出bin数 */
    void transformCompleted(int bins);

private:
    /** @brief 复数乘法辅助 */
    static void cmul(double ar, double ai, double br, double bi,
                     double& cr, double& ci);

    /** @brief 基2 FFT(就地，实部/虚部交替) */
    void fft(QVector<double>& real, QVector<double>& imag, bool inverse) const;

    /** @brief 找到>=n的最小2的幂 */
    static int nextPow2(int n);

    Stats m_stats;
    double m_timeSum = 0.0;
};
