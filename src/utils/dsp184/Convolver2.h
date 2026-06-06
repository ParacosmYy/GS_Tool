/**
 * @file Convolver2.h
 * @brief 快速卷积(重叠相加/保留FFT+分段IR实时流) — Fast Convolution via Overlap-Add/Save FFT with Partitioned IR for Real-Time Streaming
 *
 * 功能: 实现快速卷积算法，支持重叠相加/重叠保留FFT方法、
 *       分段脉冲响应处理和实时流式卷积。
 *
 * 协作: FftEngine(FFT引擎) / FIRFilter4(FIR滤波器) / AudioBuffer(音频缓冲)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 快速卷积器(重叠相加/保留FFT+分段IR)
 */
class Convolver2 : public QObject {
    Q_OBJECT

public:
    /** @brief Convolution method */
    enum class Method { OverlapAdd, OverlapSave };

    /** @brief 运行统计 */
    struct Stats {
        quint64 totalBlocks = 0;
        int blockSize = 0;
        int irLength = 0;
        int fftSize = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit Convolver2(QObject *parent = nullptr);
    ~Convolver2() override;

    void setMethod(Method method);
    void setBlockSize(int size);

    /** @brief 设置脉冲响应 */
    void setImpulseResponse(const QVector<double>& ir);

    /** @brief 处理单个数据块(流式) */
    QVector<double> processBlock(const QVector<double>& input);

    /** @brief 一次性卷积 */
    QVector<double> convolve(const QVector<double>& input,
                              const QVector<double>& ir) const;

    /** @brief 重置流式状态 */
    void reset();

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void blockProcessed(int blockSize, double timeMs);

private:
    Method m_method = Method::OverlapAdd;
    int m_blockSize = 256;

    QVector<double> m_ir;
    QVector<double> m_overlapBuffer;
    QVector<double> m_tail;

    // Partitioned IR in frequency domain
    QVector<QVector<double>> m_irPartRe;
    QVector<QVector<double>> m_irPartIm;

    Stats m_stats;
    double m_timeSum = 0.0;

    int m_fftSize = 0;
    int m_numPartitions = 0;

    /** @brief Next power of 2 */
    int nextPow2(int n) const;

    /** @brief In-place radix-2 FFT */
    void fft(QVector<double>& re, QVector<double>& im, bool inverse) const;

    /** @brief Complex multiply: (a+bi)*(c+di) */
    void complexMul(double ar, double ai, double br, double bi,
                    double& cr, double& ci) const;

    /** @brief Partition IR into frequency-domain blocks */
    void partitionIR();

    /** @brief Overlap-add convolution block */
    QVector<double> overlapAdd(const QVector<double>& input);

    /** @brief Overlap-save convolution block */
    QVector<double> overlapSave(const QVector<double>& input);
};
