/**
 * @file DistributedArithmetic5.h
 * @brief 分布式算术FIR滤波器(查找表求和+定点运算) — Distributed Arithmetic FIR Filter with LUT-Based Sum-of-Products for Fixed-Point Arithmetic
 *
 * 功能: 实现分布式算术FIR滤波器，支持查找表(LUT)预计算、
 *       位串行/位并行求和、定点Q格式和流水线MAC替换。
 *
 * 协作: FIRFilter3(基本FIR) / IIRFilter4(IIR滤波) / CICFilter3(CIC抽取)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 分布式算术FIR滤波器(LUT求和+定点运算)
 */
class DistributedArithmetic5 : public QObject {
    Q_OBJECT

public:
    /** @brief 运行统计 */
    struct Stats {
        quint64 totalFilterOps = 0;
        int filterOrder = 0;
        int lutEntries = 0;
        int fracBits = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit DistributedArithmetic5(QObject *parent = nullptr);
    ~DistributedArithmetic5() override;

    /** @brief 设置FIR系数(浮点) */
    void setCoefficients(const QVector<double>& coeffs);

    /** @brief 设置定点Q格式的小数位数 */
    void setFractionalBits(int bits);

    /** @brief 处理单个定点样本(返回定点结果) */
    qint32 processSample(qint32 sample);

    /** @brief 批量处理定点样本 */
    QVector<qint32> processBlock(const QVector<qint32>& samples);

    /** @brief 查找表内容 */
    QVector<qint32> lutTable() const { return m_lut; }

    /** @brief 浮点滤波(用于验证) */
    QVector<double> filterFloat(const QVector<double>& input) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void filterCompleted(int numSamples, double timeMs);

private:
    int m_fracBits = 15;       // Q1.15 format
    QVector<qint32> m_lut;     // Precomputed LUT
    QVector<qint32> m_coeffs;  // Fixed-point coefficients
    QVector<double> m_fCoeffs; // Floating-point coefficients
    QVector<qint32> m_shiftReg;// Shift register for input history
    int m_order = 0;
    int m_regIndex = 0;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Build LUT from coefficients */
    void buildLut();

    /** @brief Convert float coefficient to fixed-point */
    qint32 toFixedPoint(double val) const;

    /** @brief Convert fixed-point back to float */
    double fromFixedPoint(qint32 val) const;
};
