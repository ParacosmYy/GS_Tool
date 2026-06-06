/**
 * @file Goertzel4.h
 * @brief Goertzel算法(单频DFT+滑动窗口变体) — Goertzel Algorithm for Single-Bin DFT with Sliding Window Variant
 *
 * 功能: 实现Goertzel算法用于单频率DFT计算，支持标准单次计算和
 *       滑动窗口连续更新变体。
 *
 * 协作: DftEngine(DFT引擎) / FftEngine(FFT引擎) / DistributedArithmetic4(DA滤波)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief Goertzel单频DFT处理器
 */
class Goertzel4 : public QObject {
    Q_OBJECT

public:
    /** @brief 运行统计 */
    struct Stats {
        quint64 totalComputations = 0;     ///< 累计计算次数
        int blockSize = 0;                 ///< 当前块大小
        double targetFrequency = 0.0;      ///< 目标频率
        double lastMagnitude = 0.0;        ///< 最近幅度
        double lastPhase = 0.0;            ///< 最近相位
        double avgProcessingTimeMs = 0.0;  ///< 平均耗时(ms)
    };

    explicit Goertzel4(QObject *parent = nullptr);
    ~Goertzel4() override;

    /** @brief 设置目标频率(Hz) */
    void setTargetFrequency(double freq);

    /** @brief 设置采样率(Hz) */
    void setSampleRate(double rate);

    /** @brief 设置块大小(N) */
    void setBlockSize(int N);

    /**
     * @brief 对数据块执行Goertzel计算
     * @param samples 输入采样(长度>=blockSize)
     * @return {magnitude, phase} 幅度和相位
     */
    QPair<double, double> compute(const QVector<double>& samples);

    /**
     * @brief 滑动窗口Goertzel: 每次推入一个采样
     * @param sample 新采样值
     * @return {magnitude, phase}
     */
    QPair<double, double> processSample(double sample);

    /** @brief 批量滑动窗口处理 */
    QVector<QPair<double, double>> processSliding(const QVector<double>& samples);

    /** @brief 多频率Goertzel */
    QVector<QPair<double, double>> computeMultiFreq(
        const QVector<double>& samples,
        const QVector<double>& frequencies);

    void resetSliding();
    void reset();
    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void computationCompleted(double magnitude, double phase);
    void slidingUpdated(double magnitude, double phase);

private:
    /** @brief 计算Goertzel系数 */
    void updateCoefficient();

    /** @brief 标准Goertzel处理 */
    QPair<double, double> goertzelBlock(const QVector<double>& samples) const;

    double m_targetFreq = 1000.0;
    double m_sampleRate = 44100.0;
    int m_blockSize = 512;

    double m_coeff = 0.0;           ///< Goertzel系数 2*cos(2*pi*k/N)
    double m_coeff1 = 0.0;         ///< sin部分
    int m_binIndex = 0;            ///< 目标频率对应的DFT bin

    /* Sliding window state */
    double m_s1 = 0.0;             ///< Goertzel状态s[n-1]
    double m_s2 = 0.0;             ///< Goertzel状态s[n-2]
    int m_sampleCount = 0;         ///< 滑动窗口已处理采样数
    QVector<double> m_circularBuf; ///< 环形缓冲(用于减去旧采样)
    int m_bufPos = 0;              ///< 缓冲写位置
    QVector<double> m_prevS1;      ///< 前一轮s1历史
    QVector<double> m_prevS2;      ///< 前一轮s2历史

    Stats m_stats;
    double m_timeSum = 0.0;
};
