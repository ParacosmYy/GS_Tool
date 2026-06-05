/**
 * @file FftConvolution.h
 * @brief FFT快速卷积引擎 — 基于重叠相加法的频域卷积
 *
 * 功能: 利用FFT将时域卷积转化为频域乘法，支持长信号与
 *       滤波器核的高效卷积运算，采用重叠相加法处理分块。
 *
 * 协作: DigitalFilter(数字滤波) / WaveformGenerator(波形生成)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QComplexDouble>

/**
 * @brief FFT快速卷积 — 重叠相加法(Overlap-Add)
 */
class FftConvolution : public QObject {
    Q_OBJECT

public:
    /** @brief 卷积模式 */
    enum class Mode {
        Full,       ///< 完整卷积 (lenA + lenB - 1)
        Same,       ///< 等长卷积 (max(lenA, lenB))
        Valid       ///< 有效卷积 (max(lenA,lenB)-min(lenA,lenB)+1)
    };
    Q_ENUM(Mode)

    /** @brief 统计 */
    struct Stats {
        int totalConvolutions = 0;          ///< 累计卷积次数
        int totalBlocksProcessed = 0;       ///< 累计处理块数
        int totalFftsExecuted = 0;          ///< 累计FFT执行次数
        double avgProcessingTimeMs = 0.0;   ///< 平均处理耗时(ms)
        quint64 totalSamplesProcessed = 0;  ///< 累计处理采样点数
    };

    explicit FftConvolution(QObject* parent = nullptr);

    /** @brief 设置卷积模式 @param mode 模式 */
    void setMode(Mode mode);

    /** @brief 设置分块大小(0=自动) @param size 块大小 */
    void setBlockSize(int size);

    /**
     * @brief 执行卷积
     * @param signal 输入信号
     * @param kernel 卷积核
     * @return 卷积结果
     */
    QVector<double> convolve(const QVector<double>& signal,
                             const QVector<double>& kernel);

    /**
     * @brief 重叠相加法卷积(流式)
     * @param signal 长输入信号
     * @param kernel 短卷积核
     * @return 完整卷积结果
     */
    QVector<double> overlapAdd(const QVector<double>& signal,
                               const QVector<double>& kernel);

    /**
     * @brief 计算互相关(频域方法)
     * @param a 信号A
     * @param b 信号B
     * @return 互相关结果
     */
    QVector<double> crossCorrelate(const QVector<double>& a,
                                   const QVector<double>& b);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 卷积完成 @param outputSize 输出长度 @param timeMs 耗时 */
    void convolutionComplete(int outputSize, double timeMs);

private:
    /** @brief 基2 FFT(就地) @param data 复数数组 @param inverse 是否逆变换 */
    void fft(QVector<QComplexDouble>& data, bool inverse = false);

    /** @brief 计算下一个2的幂 @param n 输入值 @return >=n的最小2幂 */
    static int nextPowerOf2(int n);

    /** @brief 裁剪输出到指定模式 @param full 完整结果 @param sigLen @param kerLen */
    QVector<double> trimOutput(const QVector<double>& full,
                               int sigLen, int kerLen) const;

    Mode m_mode = Mode::Full;       ///< 卷积模式
    int m_blockSize = 0;            ///< 分块大小(0=自动)
    Stats m_stats;                  ///< 统计信息
    double m_timeSum = 0.0;         ///< 累计耗时
};
