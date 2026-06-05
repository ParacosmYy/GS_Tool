/**
 * @file ShortTimeFourier.h
 * @brief 短时傅里叶变换(STFT) — Short-Time Fourier Transform
 *
 * 功能: 支持Hanning/Hamming/Blackman/Rectangular窗函数，可配置帧长、
 *       帧移(重叠)和FFT点数。输出复数频谱矩阵(时间帧×频率bin)。
 *
 * 协作: FftEngine(底层FFT) / Spectrogram(语图可视化) / MelFilterbank(梅尔滤波)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 短时傅里叶变换(STFT)处理器
 */
class ShortTimeFourier : public QObject {
    Q_OBJECT

public:
    /** @brief 窗函数类型 */
    enum class WindowType {
        Hanning,        ///< 汉宁窗
        Hamming,        ///< 汉明窗
        Blackman,       ///< 布莱克曼窗
        Rectangular     ///< 矩形窗
    };

    /** @brief 复数表示 */
    struct Complex {
        double real = 0.0;
        double imag = 0.0;
        double magnitude() const { return qSqrt(real * real + imag * imag); }
        double phase() const { return qAtan2(imag, real); }
    };

    /** @brief 运行统计 */
    struct Stats {
        quint64 totalTransforms = 0;        ///< 累计STFT变换次数
        quint64 totalFrames = 0;            ///< 累计处理帧数
        double avgProcessingTimeMs = 0.0;   ///< 平均处理耗时(ms)
        double peakFrequency = 0.0;         ///< 峰值频率(Hz)
    };

    explicit ShortTimeFourier(QObject* parent = nullptr);

    /**
     * @brief 设置窗函数类型
     * @param type 窗函数
     */
    void setWindowType(WindowType type);

    /**
     * @brief 设置帧长度(采样点数)
     * @param size 帧长，推荐2的幂
     */
    void setFrameSize(int size);

    /**
     * @brief 设置帧移(重叠步长)
     * @param hop 帧移采样点数，小于frameSize即有重叠
     */
    void setHopSize(int hop);

    /**
     * @brief 设置FFT点数
     * @param nfft FFT点数，>= frameSize，自动补零
     */
    void setFftSize(int nfft);

    /**
     * @brief 设置采样率(用于频率计算)
     * @param rate 采样率(Hz)
     */
    void setSampleRate(double rate);

    /**
     * @brief 执行STFT变换
     * @param signal 输入时域信号
     * @return 频谱矩阵(帧数×频率bin数)，每帧为复数向量
     */
    QVector<QVector<Complex>> transform(const QVector<double>& signal);

    /**
     * @brief 执行逆STFT(ISTFT)重建信号
     * @param spectrogram STFT频谱矩阵
     * @return 重建的时域信号
     */
    QVector<double> inverseTransform(const QVector<QVector<Complex>>& spectrogram);

    /**
     * @brief 获取幅度谱矩阵
     * @param spectrogram 复数频谱
     * @return 幅度谱(帧数×频率bin数)
     */
    QVector<QVector<double>> magnitudeSpectrum(const QVector<QVector<Complex>>& spectrogram) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief STFT完成 @param frames 帧数 @param bins 频率bin数 */
    void transformCompleted(int frames, int bins);

private:
    /** @brief 生成窗函数 */
    QVector<double> generateWindow(int size) const;

    /** @brief 基2 FFT */
    void fft(QVector<Complex>& x) const;
    /** @brief 基2 IFFT */
    void ifft(QVector<Complex>& x) const;

    WindowType m_windowType = WindowType::Hanning;
    int m_frameSize = 1024;
    int m_hopSize = 512;
    int m_nfft = 1024;
    double m_sampleRate = 44100.0;

    Stats m_stats;
    double m_timeSum = 0.0;
};
