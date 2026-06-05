/**
 * @file BiquadFilter.h
 * @brief 双二阶IIR滤波器 — 音频EQ基础模块
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 双二阶(Biquad)IIR滤波器
 * 支持低通/高通/带通/带阻/峰值/低架/高架等类型
 * 使用直接I型转置结构(Direct Form I Transposed)
 */
class BiquadFilter : public QObject
{
    Q_OBJECT

public:
    /** @brief 滤波器类型 */
    enum FilterType {
        LowPass,     ///< 低通
        HighPass,    ///< 高通
        BandPass,    ///< 带通(恒定 skirts)
        Notch,       ///< 陷波(带阻)
        AllPass,     ///< 全通
        Peaking,     ///< 峰值(参数EQ)
        LowShelf,    ///< 低架
        HighShelf    ///< 高架
    };

    /** @brief 统计信息 */
    struct Stats {
        int totalSamplesProcessed = 0;    ///< 累计处理采样数
        int totalFiltersDesigned = 0;     ///< 累计设计滤波器次数
        double avgProcessingTimeMs = 0.0;
    };

    explicit BiquadFilter(QObject* parent = nullptr);

    /** @brief 设计滤波器 @param type 类型 @param freq 中心频率(Hz) @param sampleRate 采样率 @param Q Q因子 @param gainDB 增益(dB, 仅Peak/Shelf) */
    void design(FilterType type, double freq, double sampleRate,
                double Q = 0.7071, double gainDB = 0.0);

    /** @brief 处理单个采样 @param input 输入采样 @return 滤波后采样 */
    double process(double input);

    /** @brief 批量处理 @param input 输入缓冲区 @return 滤波后缓冲区 */
    QVector<double> processBuffer(const QVector<double>& input);

    /** @brief 计算频率响应 @param freqs 频率数组 @param sampleRate 采样率 @return (幅度数组, 相位数组) */
    QPair<QVector<double>, QVector<double>> frequencyResponse(
        const QVector<double>& freqs, double sampleRate) const;

    /** @brief 获取当前系数(用于显示/存储) @return (b0,b1,b2,a1,a2) */
    QVector<double> coefficients() const;

    /** @brief 重置内部状态(清空延迟线) */
    void reset();

    /** @brief 获取滤波器类型名 */
    QString filterTypeName() const;

    /** @brief 获取当前设计参数 */
    FilterType currentType() const { return m_type; }
    double currentFreq() const { return m_freq; }
    double currentQ() const { return m_Q; }
    double currentGain() const { return m_gainDB; }

    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 滤波器设计完成 @param type 类型 @param freq 频率 */
    void filterDesigned(int type, double freq);

private:
    /** @brief 应用Audio EQ Cookbook公式 */
    void computeCookbook(FilterType type, double w0, double alpha, double A);

    double m_b0 = 1.0, m_b1 = 0.0, m_b2 = 0.0;  ///< 分子系数
    double m_a1 = 0.0, m_a2 = 0.0;                ///< 分母系数(a0归一化)
    double m_z1 = 0.0, m_z2 = 0.0;                ///< 延迟线状态

    FilterType m_type = LowPass;
    double m_freq = 1000.0;
    double m_Q = 0.7071;
    double m_gainDB = 0.0;

    Stats m_stats;
    double m_timeSum = 0.0;
};
