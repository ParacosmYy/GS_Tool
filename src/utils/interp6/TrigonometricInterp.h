/**
 * @file TrigonometricInterp.h
 * @brief 三角插值引擎 — 基于FFT的三角/周期函数插值
 *
 * 功能: 对等间距采样数据进行三角多项式插值，利用FFT实现
 *       高效频域计算，支持周期信号重建与任意点插值。
 *
 * 协作: DataInterpolator(插值) / SpectrumAnalyzer(频谱分析)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QComplexDouble>

/**
 * @brief 三角插值 — FFT驱动的周期信号插值
 */
class TrigonometricInterp : public QObject {
    Q_OBJECT

public:
    /** @brief 统计 */
    struct Stats {
        int totalInterpolations = 0;            ///< 累计插值次数
        int totalPointsGenerated = 0;           ///< 累计生成点数
        int totalFftsExecuted = 0;              ///< 累计FFT执行次数
        double avgProcessingTimeMs = 0.0;       ///< 平均处理耗时(ms)
        quint64 totalSamplesProcessed = 0;      ///< 累计处理采样点数
    };

    /** @brief 三角插值模式 */
    enum class Mode {
        DFT,            ///< 标准DFT插值(任意长度)
        FFT,            ///< FFT加速(2幂长度)
        SlowDFT         ///< 直接DFT(慢但精确)
    };
    Q_ENUM(Mode)

    /** @brief 频谱信息 */
    struct SpectrumInfo {
        QVector<double> frequencies;    ///< 频率轴
        QVector<double> magnitudes;     ///< 幅度谱
        QVector<double> phases;         ///< 相位谱
        double dcComponent = 0.0;       ///< 直流分量
        double dominantFreq = 0.0;      ///< 主频率
    };

    explicit TrigonometricInterp(QObject* parent = nullptr);

    /** @brief 设置插值模式 @param mode 模式 */
    void setMode(Mode mode);

    /**
     * @brief 对数据进行三角插值
     * @param data 等间距采样数据
     * @param outputSize 输出点数(0=2倍输入)
     * @return 插值后的数据
     */
    QVector<double> interpolate(const QVector<double>& data,
                                int outputSize = 0);

    /**
     * @brief 在指定位置插值求值
     * @param data 采样数据
     * @param positions 归一化位置[0,1)
     * @return 插值结果
     */
    QVector<double> interpolateAt(const QVector<double>& data,
                                  const QVector<double>& positions);

    /**
     * @brief 提取频谱信息
     * @param data 采样数据
     * @param sampleRate 采样率
     * @return 频谱信息
     */
    SpectrumInfo analyzeSpectrum(const QVector<double>& data,
                                 double sampleRate = 1.0);

    /**
     * @brief 去除趋势/直流分量后插值
     * @param data 原始数据
     * @param outputSize 输出点数
     * @return 去趋势后的插值结果
     */
    QVector<double> detrendedInterpolate(const QVector<double>& data,
                                         int outputSize = 0);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 插值完成 @param inputSize 输入大小 @param outputSize 输出大小 */
    void interpolationComplete(int inputSize, int outputSize);

private:
    /** @brief 基2 FFT @param data 复数数据 @param inverse 逆变换 */
    void fft(QVector<QComplexDouble>& data, bool inverse = false);

    /** @brief 慢速DFT @param data 复数数据 @param inverse 逆变换 */
    void slowDft(QVector<QComplexDouble>& data, bool inverse = false);

    /** @brief 下一个2的幂 @param n 输入 @return 2幂 */
    static int nextPowerOf2(int n);

    /** @brief 去除线性趋势 @param data 数据 @return 去趋势数据 */
    QVector<double> detrend(const QVector<double>& data) const;

    Mode m_mode = Mode::FFT;        ///< 插值模式
    Stats m_stats;                  ///< 统计信息
    double m_timeSum = 0.0;         ///< 累计耗时
};
