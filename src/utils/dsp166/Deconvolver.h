/**
 * @file Deconvolver.h
 * @brief 维纳反卷积(正则化逆滤波+频域处理) — Wiener Deconvolution with Regularized Inverse Filter and Frequency-Domain Processing
 *
 * 功能: 实现维纳反卷积算法，通过正则化逆滤波在频域恢复退化信号，
 *       支持自定义PSF核、噪声功率谱估计和正则化参数调节。
 *
 * 协作: HarmonicAnalyzer(频谱分析) / FftEngine(FFT) / WalshHadamard(变换)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 维纳反卷积处理器
 */
class Deconvolver : public QObject {
    Q_OBJECT

public:
    /** @brief 运行统计 */
    struct Stats {
        quint64 totalRuns = 0;           ///< 累计运行次数
        double avgProcessingTimeMs = 0.0;///< 平均处理耗时(ms)
        double lastSNR = 0.0;           ///< 最近一次信噪比估计
        int lastSignalLength = 0;        ///< 最近一次信号长度
    };

    explicit Deconvolver(QObject* parent = nullptr);
    ~Deconvolver() override;

    /** @brief 设置正则化参数(lambda)，越大越平滑 */
    void setRegularization(double lambda);

    /** @brief 设置噪声功率谱密度 */
    void setNoisePower(double power);

    /**
     * @brief 执行维纳反卷积
     * @param signal 观测信号(退化后)
     * @param psf 点扩展函数(模糊核)
     * @return 恢复后的信号
     */
    QVector<double> deconvolve(const QVector<double>& signal,
                               const QVector<double>& psf);

    /**
     * @brief 从观测信号估计噪声功率
     * @param signal 观测信号
     * @return 估计的噪声方差
     */
    double estimateNoise(const QVector<double>& signal) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 反卷积完成 @param length 输出信号长度 */
    void deconvolutionCompleted(int length);

private:
    /** @brief FFT(基2 Cooley-Tukey) */
    void fft(QVector<double>& real, QVector<double>& imag, bool inverse) const;

    /** @brief 补零到2的幂 */
    static int nextPowerOf2(int n);

    double m_lambda = 1e-3;
    double m_noisePower = 1e-6;

    Stats m_stats;
    double m_timeSum = 0.0;
};
