/**
 * @file ZoomFFT.h
 * @brief Zoom FFT — 频带选择式高分辨率窄带频谱分析
 *
 * 功能: 实现Zoom FFT(Chirp-Z变换)窄带高分辨率频谱分析，
 *       通过频移+低通+抽取+基带FFT实现指定频带的精细分析，
 *       适用于振动分析、雷达信号处理、通信频谱监测。
 *
 * 协作: SpectrumAnalyzer(频谱分析) / AdaptiveFilter2(滤波)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

#include <vector>
#include <complex>

/**
 * @brief Zoom FFT — 频带选择式高分辨率频谱分析
 */
class ZoomFFT : public QObject {
    Q_OBJECT

public:
    /** @brief 分析配置 */
    struct Parameters {
        double sampleRate = 44100.0;      ///< 采样率(Hz)
        double centerFreq = 1000.0;       ///< 分析频带中心频率(Hz)
        double bandwidth = 200.0;         ///< 分析频带宽度(Hz)
        int fftSize = 1024;               ///< 基带FFT大小
        int decimationFactor = 4;         ///< 抽取因子
        int filterOrder = 64;             ///< 低通滤波器阶数
        double filterRipple = 0.1;        ///< 低通滤波器纹波(dB)
        double filterAttenuation = 60.0;  ///< 低通滤波器阻带衰减(dB)
    };

    /** @brief 频谱结果 */
    struct SpectrumResult {
        QVector<double> frequencies;      ///< 频率轴(Hz)
        QVector<double> magnitudes;       ///< 幅度谱(dB或线性)
        QVector<double> phases;           ///< 相位谱(rad)
        double resolutionHz = 0.0;        ///< 频率分辨率(Hz)
        double effectiveBandwidth = 0.0;  ///< 有效分析带宽(Hz)
        int actualDecimation = 0;         ///< 实际抽取因子
        bool success = false;            ///< 是否成功
    };

    /** @brief 操作统计 */
    struct Stats {
        quint64 totalAnalyses = 0;        ///< 累计分析次数
        quint64 totalSamplesProcessed = 0; ///< 累计处理采样数
        quint64 totalFFTsComputed = 0;     ///< 累计FFT计算次数
        double  avgProcessingTimeMs = 0.0; ///< 平均处理耗时(ms)
    };

    /**
     * @brief 构造函数
     * @param parent 父对象
     */
    explicit ZoomFFT(QObject* parent = nullptr);

    /** @brief 析构函数 */
    ~ZoomFFT() override;

    // ── 配置 ──

    /** @brief 设置分析参数 @param params 参数 */
    void setParameters(const Parameters& params);

    /** @brief 获取当前参数 @return 参数 */
    Parameters parameters() const;

    // ── 频谱分析 ──

    /**
     * @brief Zoom FFT分析(实信号)
     * @param samples 输入采样
     * @return 频谱结果
     */
    SpectrumResult analyze(const QVector<double>& samples);

    /**
     * @brief Zoom FFT分析(复信号)
     * @param iSamples 同相分量
     * @param qSamples 正交分量
     * @return 频谱结果
     */
    SpectrumResult analyzeComplex(const QVector<double>& iSamples,
                                  const QVector<double>& qSamples);

    /**
     * @brief 仅执行频移+低通(不计算FFT)
     * @param samples 输入采样
     * @return 滤波后的基带信号
     */
    QVector<double> frequencyShift(const QVector<double>& samples) const;

    // ── 滤波器设计 ──

    /**
     * @brief 设计抽取低通滤波器
     * @param cutoff 截止频率(Hz)
     * @param sampleRate 采样率(Hz)
     * @param order 阶数
     * @return 滤波器系数
     */
    QVector<double> designLowpass(double cutoff, double sampleRate,
                                  int order) const;

    /** @brief 设计默认滤波器(使用当前参数) */
    void designDefaultFilter();

    // ── 辅助 ──

    /** @brief 计算有效频率分辨率 @return 分辨率(Hz) */
    double effectiveResolution() const;

    /** @brief 计算所需最小输入长度 @return 最小采样数 */
    int minimumInputLength() const;

    // ── 统计 ──

    /** @brief 获取统计 @return 统计 */
    Stats stats() const;

    /** @brief 重置统计 */
    void resetStatistics();

signals:
    /** @brief 分析完成 @param resolution 频率分辨率 @param bandwidth 分析带宽 */
    void analysisCompleted(double resolution, double bandwidth);

    /** @brief 进度通知 @param stage 当前阶段 @param percent 百分比 */
    void progressChanged(const QString& stage, int percent);

private:
    /**
     * @brief 频移(复数混频)
     * @param samples 输入
     * @param freq 载波频率(Hz)
     * @return (I, Q)基带信号
     */
    QPair<QVector<double>, QVector<double>> mixDown(
        const QVector<double>& samples, double freq) const;

    /**
     * @brief FIR低通滤波
     * @param signal 输入信号
     * @param coeffs 滤波器系数
     * @return 滤波后信号
     */
    QVector<double> applyFIR(const QVector<double>& signal,
                             const QVector<double>& coeffs) const;

    /**
     * @brief 抽取(降采样)
     * @param signal 输入信号
     * @param factor 抽取因子
     * @return 抽取后信号
     */
    QVector<double> decimate(const QVector<double>& signal,
                             int factor) const;

    /**
     * @brief 基带FFT(Cooley-Tukey)
     * @param re 实部
     * @param im 虚部
     */
    void computeFFT(std::vector<double>& re, std::vector<double>& im) const;

    /**
     * @brief Kaiser窗函数值
     * @param n 采样索引
     * @param N 总长度
     * @param beta 形状参数
     * @return 窗值
     */
    double kaiserWindow(int n, int N, double beta) const;

    /**
     * @brief 计算Kaiser窗beta参数
     * @param attenuation 阻带衰减(dB)
     * @return beta
     */
    double kaiserBeta(double attenuation) const;

    /**
     * @brief 零阶修正Bessel函数I0
     * @param x 参数
     * @return I0(x)
     */
    double besselI0(double x) const;

    Parameters m_params;                 ///< 分析参数
    QVector<double> m_filterCoeffs;      ///< 预计算低通滤波器系数

    Stats m_stats;                       ///< 操作统计
    double m_timeSum = 0.0;              ///< 累计耗时
};
