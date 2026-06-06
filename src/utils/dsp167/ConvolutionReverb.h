/**
 * @file ConvolutionReverb.h
 * @brief 卷积混响(分块Overlap-Save FFT实时IR处理) — Convolution Reverb with Partitioned Overlap-Save FFT for Real-Time IR Processing
 *
 * 功能: 实现分块卷积混响，使用Overlap-Save方法将长脉冲响应(IR)分块处理，
 *       通过FFT实现频域快速卷积，适用于实时音频流处理。
 *
 * 协作: WalshHadamard(正交变换) / FftEngine(FFT引擎) / IirFilter(IIR滤波)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 卷积混响处理器
 */
class ConvolutionReverb : public QObject {
    Q_OBJECT

public:
    /** @brief 运行统计 */
    struct Stats {
        quint64 totalBlocks = 0;           ///< 累计处理块数
        double avgProcessingTimeMs = 0.0;  ///< 平均块处理耗时(ms)
        int blockSize = 0;                 ///< 当前块大小
        int irLength = 0;                  ///< 当前IR长度
    };

    explicit ConvolutionReverb(QObject* parent = nullptr);
    ~ConvolutionReverb() override;

    /** @brief 设置处理块大小(须为2的幂) */
    void setBlockSize(int size);

    /**
     * @brief 加载脉冲响应(IR)
     * @param ir 脉冲响应数据
     */
    void loadImpulseResponse(const QVector<double>& ir);

    /**
     * @brief 处理一个音频块(Overlap-Save卷积)
     * @param input 输入音频块(长度=blockSize)
     * @return 输出音频块
     */
    QVector<double> process(const QVector<double>& input);

    /** @brief 重置内部状态(清除延迟线) */
    void reset();

    /** @brief 设置干/湿混合比(0.0=全干, 1.0=全湿) */
    void setWetDryMix(double wet);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief IR加载完成 @param length IR长度 */
    void impulseResponseLoaded(int length);
    /** @brief 块处理完成 @param blockSize 块大小 */
    void blockProcessed(int blockSize);

private:
    /** @brief 执行FFT(就地) */
    void fft(QVector<double>& real, QVector<double>& imag);

    /** @brief 执行IFFT(就地) */
    void ifft(QVector<double>& real, QVector<double>& imag);

    /** @brief 计算倒序索引 */
    static int bitReverse(int x, int log2n);

    /** @brief 将IR分块并预计算FFT */
    void partitionIR();

    int m_blockSize = 512;
    double m_wetDryMix = 0.5;

    /* Partitioned IR in frequency domain: each partition is (real, imag) */
    QVector<QVector<double>> m_irRealParts;
    QVector<QVector<double>> m_irImagParts;
    int m_partitionCount = 0;
    int m_fftSize = 0;

    /* Overlap-save history buffer (frequency domain) */
    QVector<QVector<double>> m_historyReal;
    QVector<QVector<double>> m_historyImag;

    /* Previous input tail for overlap */
    QVector<double> m_overlapTail;

    Stats m_stats;
    double m_timeSum = 0.0;
};
