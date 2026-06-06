/**
 * @file DistributedArithmetic4.h
 * @brief 分布式算术FIR滤波器(预计算LUT无乘法器卷积) — Distributed Arithmetic FIR Filter with Precomputed LUT for Multiplierless Convolution
 *
 * 功能: 实现分布式算术FIR滤波，通过预计算查找表(LUT)实现无乘法器卷积，
 *       支持串行位处理和并行位切片两种模式。
 *
 * 协作: FftFilter(FFT滤波) / FirFilter(标准FIR) / IirFilter(IIR滤波)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 分布式算术FIR滤波器
 */
class DistributedArithmetic4 : public QObject {
    Q_OBJECT

public:
    /** @brief 运行模式 */
    enum Mode { SerialBit = 0, ParallelBit = 1 };

    /** @brief 运行统计 */
    struct Stats {
        quint64 totalProcesses = 0;       ///< 累计处理次数
        int filterOrder = 0;              ///< 滤波器阶数
        int lutSize = 0;                  ///< LUT表大小
        double avgProcessingTimeMs = 0.0; ///< 平均耗时(ms)
    };

    explicit DistributedArithmetic4(QObject *parent = nullptr);
    ~DistributedArithmetic4() override;

    /** @brief 设置FIR系数并构建LUT */
    void setCoefficients(const QVector<double>& coeffs);

    void setMode(Mode mode);
    void setInputBitWidth(int bits);

    /**
     * @brief 处理输入采样序列
     * @param input 输入采样(浮点，会被量化到固定位宽)
     * @return 滤波后输出
     */
    QVector<double> process(const QVector<double>& input);

    /** @brief 单采样点处理 */
    double processOne(double sample);

    /** @brief 获取LUT表内容(调试用) */
    QVector<double> lutTable() const;

    /** @brief 获取FIR系数 */
    QVector<double> coefficients() const;

    void reset();
    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void filterInitialized(int order, int lutSize);
    void processingCompleted(int samples);

private:
    /** @brief 构建DA查找表 */
    void buildLUT();

    /** @brief 量化浮点输入到固定位宽整数 */
    qint32 quantize(double sample) const;

    /** @brief 串行位处理: 每次处理1位 */
    double serialDA(qint32* shiftReg) const;

    /** @brief 并行位处理: 位切片查表 */
    double parallelDA(qint32* shiftReg) const;

    Mode m_mode = SerialBit;
    int m_bitWidth = 16;           ///< 输入位宽

    QVector<double> m_coeffs;      ///< FIR系数
    QVector<double> m_lut;         ///< 预计算LUT表(2^N条目)
    QVector<qint32> m_shiftReg;    ///< 移位寄存器
    int m_order = 0;               ///< 滤波器阶数
    int m_writePos = 0;            ///< 移位寄存器写位置

    double m_scaleFactor = 0.0;    ///< 量化缩放因子

    Stats m_stats;
    double m_timeSum = 0.0;
};
