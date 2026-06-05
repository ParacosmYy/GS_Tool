/**
 * @file ShortTimeFFT.h
 * @brief 短时傅里叶变换(STFT) — 时频分析引擎
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 短时傅里叶变换引擎
 * 支持多种窗函数、可调步长、幅度/相位/功率谱输出
 */
class ShortTimeFFT : public QObject
{
    Q_OBJECT

public:
    /** @brief 窗函数类型 */
    enum WindowType {
        Rectangular,   ///< 矩形窗
        Hann,          ///< Hann窗
        Hamming,       ///< Hamming窗
        Blackman,      ///< Blackman窗
        Kaiser         ///< Kaiser窗
    };

    /** @brief 统计信息 */
    struct Stats {
        int totalTransforms = 0;          ///< 累计变换次数
        int totalFramesProcessed = 0;     ///< 累计处理帧数
        int totalSamplesProcessed = 0;    ///< 累计处理采样数
        double avgProcessingTimeMs = 0.0;
    };

    /**
     * @brief 构造函数
     * @param fftSize FFT大小(2的幂)
     * @param hopSize 步长(采样数)
     * @param windowType 窗函数类型
     * @param parent 父对象
     */
    explicit ShortTimeFFT(int fftSize = 1024, int hopSize = 256,
                          WindowType windowType = Hann, QObject* parent = nullptr);

    /** @brief 执行STFT @param signal 输入信号 @return 复数频谱矩阵(帧×频率) */
    QVector<QVector<QPair<double,double>>> transform(
        const QVector<double>& signal);

    /** @brief 仅计算幅度谱 @param signal 输入信号 @return 幅度矩阵(帧×频率) */
    QVector<QVector<double>> magnitudeSpectrum(const QVector<double>& signal);

    /** @brief 计算功率谱密度 @param signal 输入信号 @return PSD矩阵 */
    QVector<QVector<double>> powerSpectrum(const QVector<double>& signal);

    /** @brief 逆STFT(重叠相加法) @param stftFrames 复数频谱矩阵 @param totalSamples 输出采样数 @return 重建信号 */
    QVector<double> inverseTransform(
        const QVector<QVector<QPair<double,double>>>& stftFrames,
        int totalSamples);

    /** @brief 获取窗函数 @return 窗系数 */
    QVector<double> window() const { return m_window; }

    /** @brief 获取频率轴 @param sampleRate 采样率 @return 频率数组(Hz) */
    QVector<double> frequencyAxis(double sampleRate) const;

    /** @brief 获取时间轴 @param sampleRate 采样率 @param totalSamples 总采样数 @return 时间数组(s) */
    QVector<double> timeAxis(double sampleRate, int totalSamples) const;

    /** @brief 获取FFT大小 */
    int fftSize() const { return m_fftSize; }
    /** @brief 获取步长 */
    int hopSize() const { return m_hopSize; }

    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 变换完成 @param frames 帧数 @param bins 频率bin数 */
    void transformCompleted(int frames, int bins);

private:
    /** @brief 生成窗函数 */
    void generateWindow();

    /** @brief 基2 FFT(就地) @param real 实部 @param imag 虚部 @param inverse 是否逆变换 */
    void fft(QVector<double>& real, QVector<double>& imag, bool inverse) const;

    int m_fftSize;                      ///< FFT大小
    int m_hopSize;                       ///< 步长
    WindowType m_windowType;             ///< 窗函数类型
    QVector<double> m_window;            ///< 窗系数

    Stats m_stats;
    double m_timeSum = 0.0;
};
