/**
 * @file MultiRateFilter.h
 * @brief 多速率滤波器组 — 抽取/内插滤波器组实现
 *
 * 功能: 实现多速率信号处理中的抽取(decimation)和内插(interpolation)
 *       滤波器组，支持多相分解、半带滤波器、CIC滤波器。
 *       适用于采样率转换、子带编码、音频重采样。
 *
 * 协作: AdaptiveFFT(频谱分析) / AutomaticGainControl(增益控制)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QList>
#include <QPair>

/**
 * @brief 多速率滤波器组引擎
 */
class MultiRateFilter : public QObject {
    Q_OBJECT

public:
    /** @brief 统计信息 */
    struct Stats {
        int totalDecimations = 0;              ///< 累计抽取次数
        int totalInterpolations = 0;           ///< 累计内插次数
        int totalSamplesProcessed = 0;         ///< 累计处理采样点数
        double avgProcessingTimeMs = 0.0;      ///< 平均处理时间(ms)
    };

    /** @brief 滤波器类型 */
    enum FilterType {
        LowPass,       ///< 低通滤波器
        HalfBand,      ///< 半带滤波器
        CIC            ///< CIC(级联积分梳状)滤波器
    };

    /**
     * @brief 构造函数
     * @param parent 父对象
     */
    explicit MultiRateFilter(QObject* parent = nullptr);

    /**
     * @brief 设计低通FIR滤波器(窗口法)
     * @param cutoffFreq 归一化截止频率[0, 0.5]
     * @param filterLength 滤波器长度
     * @param windowType 窗类型(0=矩形, 1=Hann, 2=Hamming, 3=Blackman)
     * @return 滤波器系数
     */
    QVector<double> designLowPass(double cutoffFreq, int filterLength,
                                   int windowType = 1) const;

    /**
     * @brief 设计半带滤波器(长度必须为4k+3)
     * @param filterLength 滤波器长度
     * @return 滤波器系数
     */
    QVector<double> designHalfBand(int filterLength) const;

    /**
     * @brief 设计CIC滤波器
     * @param stageCount 级数(R)
     * @param differentialDelay 差分延迟(M, 通常1或2)
     * @param decimationFactor 抽取因子
     * @return 归一化CIC频率响应
     */
    QVector<double> designCIC(int stageCount, int differentialDelay,
                               int decimationFactor) const;

    /**
     * @brief 抽取(降低采样率)
     * @param signal 输入信号
     * @param factor 抽取因子
     * @param filter 抗混叠滤波器系数(空=自动设计)
     * @return 抽取后信号
     */
    QVector<double> decimate(const QVector<double>& signal, int factor,
                              QVector<double> filter = {});

    /**
     * @brief 内插(提高采样率)
     * @param signal 输入信号
     * @param factor 内插因子
     * @param filter 镜像抑制滤波器系数(空=自动设计)
     * @return 内插后信号
     */
    QVector<double> interpolate(const QVector<double>& signal, int factor,
                                 QVector<double> filter = {});

    /**
     * @brief 任意比率重采样(线性内插)
     * @param signal 输入信号
     * @param inputRate 输入采样率
     * @param outputRate 输出采样率
     * @return 重采样后信号
     */
    QVector<double> resample(const QVector<double>& signal,
                              double inputRate, double outputRate);

    /**
     * @brief 多相分解
     * @param filter 原始滤波器系数
     * @param factor 分解因子
     * @return 多相子滤波器列表
     */
    QList<QVector<double>> polyphaseDecompose(
        const QVector<double>& filter, int factor) const;

    /**
     * @brief 多相抽取(高效实现)
     * @param signal 输入信号
     * @param factor 抽取因子
     * @param polyphaseFilters 多相子滤波器
     * @return 抽取后信号
     */
    QVector<double> polyphaseDecimate(
        const QVector<double>& signal, int factor,
        const QList<QVector<double>>& polyphaseFilters);

    /**
     * @brief FIR滤波
     * @param signal 输入信号
     * @param coefficients 滤波器系数
     * @return 滤波后信号
     */
    static QVector<double> applyFir(const QVector<double>& signal,
                                     const QVector<double>& coefficients);

    /**
     * @brief 获取统计信息
     */
    const Stats& stats() const { return m_stats; }

    /**
     * @brief 重置统计信息
     */
    void resetStatistics();

signals:
    /**
     * @brief 抽取完成
     * @param inputLen 输入长度
     * @param outputLen 输出长度
     * @param factor 抽取因子
     */
    void decimationCompleted(int inputLen, int outputLen, int factor);

    /**
     * @brief 内插完成
     * @param inputLen 输入长度
     * @param outputLen 输出长度
     * @param factor 内插因子
     */
    void interpolationCompleted(int inputLen, int outputLen, int factor);

private:
    Stats m_stats;
    double m_timeSum = 0.0;                     ///< 处理时间累加器
};
