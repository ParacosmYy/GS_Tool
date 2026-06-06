/**
 * @file CorrelationAnalyzer.h
 * @brief 相关性分析器(FFT加速互相关/自相关+归一化系数) — Cross/Auto-Correlation with FFT Acceleration and Normalized Correlation Coefficient
 *
 * 功能: 实现互相关和自相关分析，支持FFT加速计算、归一化相关系数、
 *       滞后函数生成和峰值检测。
 *
 * 协作: SplitRadixFFT(分裂基FFT) / PeakDetector3(峰值检测) / SpectrumAnalyzer5(频谱分析)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 相关性分析器
 */
class CorrelationAnalyzer : public QObject {
    Q_OBJECT

public:
    /** @brief 相关类型 */
    enum CorrType { AutoCorrelation, CrossCorrelation };

    /** @brief 相关结果 */
    struct CorrResult {
        QVector<double> lags;          ///< 滞后值
        QVector<double> correlation;   ///< 相关函数值
        double peakLag = 0.0;          ///< 峰值滞后
        double peakValue = 0.0;        ///< 峰值
        double normalizedCoeff = 0.0;  ///< 归一化相关系数
    };

    /** @brief 运行统计 */
    struct Stats {
        quint64 totalComputations = 0;     ///< 累计计算次数
        double avgProcessingTimeMs = 0.0;  ///< 平均耗时(ms)
        int lastLength = 0;                ///< 最近信号长度
    };

    explicit CorrelationAnalyzer(QObject *parent = nullptr);
    ~CorrelationAnalyzer() override;

    void setUseFFT(bool enable);
    void setMaxLag(int maxLag);

    /**
     * @brief 计算自相关
     * @param signal 输入信号
     * @return 相关结果
     */
    CorrResult autoCorrelation(const QVector<double>& signal);

    /**
     * @brief 计算互相关
     * @param signalA 信号A
     * @param signalB 信号B
     * @return 相关结果
     */
    CorrResult crossCorrelation(const QVector<double>& signalA,
                                const QVector<double>& signalB);

    /**
     * @brief 计算归一化互相关系数(Pearson)
     * @param a 信号A
     * @param b 信号B
     * @return 相关系数[-1, 1]
     */
    static double pearsonCoeff(const QVector<double>& a,
                               const QVector<double>& b);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void computationCompleted(CorrType type, double peakLag, double coeff);

private:
    /** @brief 直接相关计算(暴力) */
    CorrResult directCorrelation(const QVector<double>& a,
                                  const QVector<double>& b,
                                  bool isAuto) const;

    /** @brief FFT加速相关计算 */
    CorrResult fftCorrelation(const QVector<double>& a,
                               const QVector<double>& b,
                               bool isAuto);

    /** @brief 找峰值 */
    static QPair<double, double> findPeak(const QVector<double>& lags,
                                           const QVector<double>& corr);

    /** @brief 基2 FFT(就地) */
    void fft2(QVector<double>& real, QVector<double>& imag) const;

    /** @brief 基2 IFFT(就地) */
    void ifft2(QVector<double>& real, QVector<double>& imag) const;

    /** @brief 下一个2的幂 */
    static int nextPow2(int n);

    bool m_useFFT = true;
    int m_maxLag = -1;

    Stats m_stats;
    double m_timeSum = 0.0;
};
