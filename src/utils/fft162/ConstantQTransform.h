/**
 * @file ConstantQTransform.h
 * @brief 常Q变换(CQT) — Constant-Q Transform for Music Analysis
 *
 * 功能: 实现常Q变换，频率轴按几何级数分布(每八度固定bins数)，
 *       适合音乐分析。支持可配置八度范围、每八度bins数和阈值。
 *
 * 协作: ShortTimeFourier(STFT) / MelFilterbank(梅尔滤波) / FftEngine(FFT核心)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 常Q变换处理器
 */
class ConstantQTransform : public QObject {
    Q_OBJECT

public:
    /** @brief 运行统计 */
    struct Stats {
        quint64 totalTransforms = 0;        ///< 累计CQT变换次数
        quint64 totalBinsComputed = 0;      ///< 累计计算的总bin数
        double avgProcessingTimeMs = 0.0;   ///< 平均处理耗时(ms)
        double minFrequency = 0.0;          ///< 最低分析频率(Hz)
        double maxFrequency = 0.0;          ///< 最高分析频率(Hz)
    };

    explicit ConstantQTransform(QObject* parent = nullptr);

    /**
     * @brief 设置采样率
     * @param rate 采样率(Hz)
     */
    void setSampleRate(double rate);

    /**
     * @brief 设置每八度的频率bin数
     * @param bins 每八度bins数(推荐12,24,36)
     */
    void setBinsPerOctave(int bins);

    /**
     * @brief 设置分析八度范围
     * @param minOctave 最低八度(如0表示C1)
     * @param maxOctave 最高八度(如7表示C8)
     */
    void setOctaveRange(int minOctave, int maxOctave);

    /**
     * @brief 设置阈值(低于最大值该比例的成分被置零)
     * @param threshold 阈值(0.0-1.0)
     */
    void setThreshold(double threshold);

    /**
     * @brief 预计算CQT核矩阵
     */
    void buildKernel();

    /**
     * @brief 执行CQT变换
     * @param signal 输入时域信号
     * @return CQT系数矩阵(bin数 × 帧数)
     */
    QVector<QVector<double>> transform(const QVector<double>& signal);

    /**
     * @brief 获取各bin的中心频率
     */
    QVector<double> centerFrequencies() const { return m_freqs; }

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief CQT完成 @param bins bin数 @param frames 帧数 */
    void transformCompleted(int bins, int frames);

private:
    /** @brief 计算最小FFT长度 */
    int computeMinFFT() const;

    /** @brief 基2 FFT(就地) */
    void fft(QVector<double>& real, QVector<double>& imag) const;

    double m_sampleRate = 44100.0;
    int m_binsPerOctave = 24;
    int m_minOctave = 1;
    int m_maxOctave = 7;
    double m_threshold = 0.005;

    int m_totalBins = 0;                ///< 总bin数
    QVector<double> m_freqs;            ///< 各bin中心频率
    QVector<QVector<double>> m_kernelReal;  ///< CQT核实部
    QVector<QVector<double>> m_kernelImag;  ///< CQT核电部
    QVector<int> m_fftLengths;          ///< 各bin的FFT长度

    Stats m_stats;
    double m_timeSum = 0.0;
};
