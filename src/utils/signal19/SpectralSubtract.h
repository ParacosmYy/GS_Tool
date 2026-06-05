/**
 * @file SpectralSubtract.h
 * @brief 谱减法降噪 — 噪声估计 + 过减 + 频谱下限 + Wiener后处理
 *
 * 功能: 实现谱减法(Spectral Subtraction)噪声抑制，支持多种噪声估计策略、
 *       过减因子控制、频谱下限防止音乐噪声、Wiener滤波后处理增强。
 *       适用于语音增强、音频降噪、通信信号清理。
 *
 * 协作: SpectrumAnalyzer(频谱分析) / DigitalFilter(滤波)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

#include <vector>

/**
 * @brief 谱减法降噪 — 噪声估计 + 过减 + 频谱下限
 */
class SpectralSubtract : public QObject {
    Q_OBJECT

public:
    /** @brief 噪声估计方法 */
    enum NoiseEstimation {
        Manual = 0,           ///< 手动指定噪声频谱
        FirstFrames = 1,      ///< 首N帧平均(静音段)
        MinTrack = 2,         ///< 最小值跟踪(频域)
        MMSE = 3              ///< MMSE最优估计
    };
    Q_ENUM(NoiseEstimation)

    /** @brief 降噪参数 */
    struct Parameters {
        int fftSize = 512;                 ///< FFT大小(2的幂)
        int hopSize = 256;                 ///< 帧移(50%重叠)
        int noiseFrames = 5;               ///< 噪声估计帧数(FirstFrames模式)
        double overSubtraction = 2.0;      ///< 过减因子alpha(1~5)
        double spectralFloor = 0.02;       ///< 频谱下限beta(0~0.5)
        NoiseEstimation noiseEstim = FirstFrames; ///< 噪声估计方法
        bool enableWiener = true;          ///< Wiener后处理
        double wienerGain = 1.0;           ///< Wiener增益系数
        bool enableSmoothing = true;       ///< 频谱平滑
        double smoothCoeff = 0.98;         ///< 平滑系数(0~1)
        double minTrackAlpha = 0.95;       ///< 最小跟踪遗忘因子
        double minTrackProb = 0.7;         ///< 噪声概率阈值
        int sampleRate = 44100;            ///< 采样率(Hz)
    };

    /** @brief 帧级分析数据 */
    struct FrameAnalysis {
        double snr = 0.0;                  ///< 帧信噪比(dB)
        double noisePower = 0.0;           ///< 帧噪声功率
        double signalPower = 0.0;          ///< 帧信号功率
        double reductionDb = 0.0;          ///< 降噪量(dB)
    };

    /** @brief 操作统计 */
    struct Stats {
        quint64 totalFramesProcessed = 0;  ///< 累计处理帧数
        quint64 totalSamplesProcessed = 0; ///< 累计处理采样数
        double  avgNoisePower = 0.0;       ///< 平均噪声功率
        double  avgSnr = 0.0;              ///< 平均信噪比(dB)
        double  avgProcessingTimeMs = 0.0; ///< 平均处理耗时(ms)
    };

    /**
     * @brief 构造函数
     * @param parent 父对象
     */
    explicit SpectralSubtract(QObject* parent = nullptr);

    /** @brief 析构函数 */
    ~SpectralSubtract() override;

    // ── 配置 ──

    /** @brief 设置降噪参数 @param params 参数 */
    void setParameters(const Parameters& params);

    /** @brief 获取当前参数 @return 参数 */
    Parameters parameters() const;

    // ── 降噪处理 ──

    /**
     * @brief 处理完整信号(自动噪声估计)
     * @param signal 输入含噪信号
     * @return 降噪后信号
     */
    QVector<double> process(const QVector<double>& signal);

    /**
     * @brief 处理完整信号(手动噪声参考)
     * @param signal 含噪信号
     * @param noiseRef 噪声参考片段
     * @return 降噪后信号
     */
    QVector<double> processWithNoiseRef(const QVector<double>& signal,
                                        const QVector<double>& noiseRef);

    /**
     * @brief 处理单帧
     * @param frame 输入帧(fftSize长)
     * @param isFirst 是否首帧(影响噪声估计)
     * @return (降噪后帧, 帧分析数据)
     */
    QPair<QVector<double>, FrameAnalysis> processFrame(
        const QVector<double>& frame, bool isFirst);

    // ── 噪声估计 ──

    /**
     * @brief 手动设置噪声频谱
     * @param magnitude 噪声幅度谱
     */
    void setNoiseSpectrum(const QVector<double>& magnitude);

    /**
     * @brief 从噪声片段估计噪声频谱
     * @param noise 噪声参考信号
     * @return 噪声幅度谱
     */
    QVector<double> estimateNoise(const QVector<double>& noise) const;

    /**
     * @brief 获取当前噪声估计
     * @return 噪声幅度谱
     */
    QVector<double> noiseSpectrum() const;

    // ── 统计 ──

    /** @brief 获取统计 @return 统计 */
    Stats stats() const;

    /** @brief 重置统计 */
    void resetStatistics();

signals:
    /** @brief 处理完成 @param numFrames 帧数 @param avgSnr 平均SNR */
    void processingCompleted(int numFrames, double avgSnr);

    /** @brief 帧处理进度 @param frame 当前帧号 @param snr 当前SNR */
    void frameProcessed(int frame, double snr);

    /** @brief 噪声估计更新 @param power 新噪声功率 */
    void noiseEstimateUpdated(double power);

private:
    /** @brief FFT(Cooley-Tukey) @param re 实部 @param im 虚部 */
    void computeFFT(std::vector<double>& re, std::vector<double>& im) const;

    /** @brief IFFT @param re 实部 @param im 虚部 */
    void computeIFFT(std::vector<double>& re, std::vector<double>& im) const;

    /** @brief 应用Hann窗 @param frame 输入帧 @return 加窗后帧 */
    QVector<double> applyWindow(const QVector<double>& frame) const;

    /** @brief Wiener滤波增益 @param signalPow 信号功率 @param noisePow 噪声功率 @return 增益 */
    QVector<double> wienerGain(const QVector<double>& signalPow,
                               const QVector<double>& noisePow) const;

    /** @brief 最小值跟踪噪声估计更新 @param magnitude 幅度谱 */
    void updateMinTrack(const QVector<double>& magnitude);

    /** @brief MMSE噪声估计更新 @param magnitude 幅度谱 */
    void updateMMSE(const QVector<double>& magnitude);

    /** @brief 重叠相加 @param frame 帧 @param output 输出缓冲 @param pos 位置 */
    void overlapAdd(const QVector<double>& frame,
                    QVector<double>& output, int pos) const;

    Parameters m_params;                 ///< 降噪参数
    QVector<double> m_noiseMagnitude;    ///< 噪声幅度谱估计
    QVector<double> m_noisePower;        ///< 噪声功率谱
    QVector<double> m_prevMagnitude;     ///< 前一帧幅度谱(平滑用)
    QVector<double> m_minTrackBuf;       ///< 最小跟踪缓冲区
    QVector<double> m_mmseNoise;         ///< MMSE噪声估计
    bool m_noiseEstimated;               ///< 噪声是否已估计
    int m_frameCount;                    ///< 已处理帧数

    Stats m_stats;                       ///< 操作统计
    double m_timeSum = 0.0;              ///< 累计耗时
};
