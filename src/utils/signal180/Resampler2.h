/**
 * @file Resampler2.h
 * @brief 采样率转换器(多相FIR抗混叠+任意比Farrow结构) — Sample Rate Converter with Polyphase FIR Anti-alias Filter and Arbitrary Ratio via Farrow Structure
 *
 * 功能: 实现采样率转换，支持多相FIR抗混叠滤波器、
 *       任意比率转换（Farrow结构）和线性/三次插值。
 *
 * 协作: FIRFilter3(FIR滤波器) / Window3(窗函数) / Analyzer3(频谱分析)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 采样率转换器(多相FIR+Farrow结构)
 */
class Resampler2 : public QObject {
    Q_OBJECT

public:
    /** @brief 插值类型 */
    enum Interpolation { Linear, Cubic, Farrow };

    /** @brief 运行统计 */
    struct Stats {
        quint64 totalResamples = 0;
        int inputRate = 0;
        int outputRate = 0;
        int inputSamples = 0;
        int outputSamples = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit Resampler2(QObject *parent = nullptr);
    ~Resampler2() override;

    void setInputRate(int rate);
    void setOutputRate(int rate);
    void setFilterTaps(int taps);
    void setInterpolation(Interpolation type);

    /** @brief 执行采样率转换 */
    QVector<double> process(const QVector<double>& input);

    /** @brief 多相分解抗混叠FIR滤波 */
    QVector<double> polyphaseFilter(const QVector<double>& input,
                                     int upFactor, int downFactor) const;

    /** @brief Farrow结构任意比率插值 */
    QVector<double> farrowInterpolate(const QVector<double>& input,
                                       double ratio) const;

    /** @brief 设计抗混叠低通FIR( sinc窗 ) */
    QVector<double> designFilter(double cutoff, int taps) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void resampleCompleted(int inSamples, int outSamples);

private:
    int m_inputRate = 44100;
    int m_outputRate = 48000;
    int m_filterTaps = 64;
    Interpolation m_interp = Farrow;

    QVector<double> m_firCoeffs;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief 三次插值系数(Farrow) */
    double farrowCoeff(const QVector<double>& y, double mu) const;
};
