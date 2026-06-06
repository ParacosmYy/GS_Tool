/**
 * @file Resampler.h
 * @brief 多相重采样器(任意比率+FIR抗混叠滤波器设计) — Polyphase Resampler with Arbitrary Ratio and FIR Anti-Aliasing Filter Design
 *
 * 功能: 实现多相结构重采样器，支持任意有理比率(L/M)、FIR抗混叠滤波器自动设计、
 *       窗函数选择和多相分解，适用于嵌入式音频采样率转换。
 *
 * 协作: FIRFilter(FIR滤波器) / WindowFunction(窗函数) / Interpolator(插值器)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 多相重采样器
 */
class Resampler : public QObject {
    Q_OBJECT

public:
    /** @brief 窗函数类型 */
    enum WindowType { Blackman, Hamming, Hann, Kaiser };

    /** @brief 运行统计 */
    struct Stats {
        quint64 totalSamplesIn = 0;       ///< 累计输入样本数
        quint64 totalSamplesOut = 0;      ///< 累计输出样本数
        double avgProcessingTimeMs = 0.0; ///< 平均耗时(ms)
        double ratio = 1.0;               ///< 当前重采样比率
    };

    explicit Resampler(QObject *parent = nullptr);
    ~Resampler() override;

    /**
     * @brief 配置重采样比率
     * @param inRate 输入采样率
     * @param outRate 输出采样率
     */
    void setRatio(double inRate, double outRate);

    /** @brief 设置滤波器长度(须为L的倍数) */
    void setFilterLength(int len);

    /** @brief 设置窗函数 */
    void setWindowType(WindowType type);

    /** @brief 设置Kaiser窗beta参数 */
    void setKaiserBeta(double beta);

    /**
     * @brief 处理采样数据
     * @param input 输入采样
     * @return 重采样输出
     */
    QVector<double> process(const QVector<double>& input);

    /**
     * @brief 单样本递推处理
     * @param sample 输入采样
     * @return 输出采样(可能为空)
     */
    QVector<double> pushSample(double sample);

    /** @brief 获取多相子滤波器 */
    QVector<QVector<double>> polyphaseFilters() const;

    /** @brief 获取抗混叠滤波器系数 */
    QVector<double> filterCoeffs() const;

    /** @brief 重置内部状态 */
    void reset();

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 处理完成 @param inCount 输入数 @param outCount 输出数 */
    void processingCompleted(quint64 inCount, quint64 outCount);

private:
    /** @brief 设计抗混叠FIR滤波器 */
    void designFilter();

    /** @brief 分解为多相子滤波器 */
    void decomposePolyphase();

    /** @brief 窗函数 */
    double windowFunc(int n, int N) const;

    /** @brief Sinc函数 */
    static double sinc(double x);

    int m_L = 1;              ///< 上采样因子
    int m_M = 1;              ///< 下采样因子
    int m_filterLen = 0;
    WindowType m_window = Blackman;
    double m_kaiserBeta = 5.0;

    QVector<double> m_filter;              ///< FIR滤波器系数
    QVector<QVector<double>> m_polyFilter; ///< 多相子滤波器
    QVector<double> m_delayLine;           ///< 延迟线
    int m_delayIdx = 0;                    ///< 延迟线写入位置
    int m_phase = 0;                       ///< 当前相位

    Stats m_stats;
    double m_timeSum = 0.0;
};
