/**
 * @file DistributedArithmetic9.h
 * @brief 分布式算术(可重构LUT+串并混合累加多速率FIR) — Distributed Arithmetic with Reconfigurable LUT and Serial-Parallel Hybrid Accumulation for Multi-Rate FIR
 *
 * 功能: 实现分布式算术(Distributed Arithmetic)FIR滤波器，使用可重构
 *       查找表(reconfigurable LUT)存储预计算系数加权和，串并混合累加
 *       (serial-parallel hybrid accumulation)支持多速率FIR滤波。
 *
 * 协作: FIRFilter8(FIR滤波) / PolyphaseFilter7(多相滤波) / CICFilter5(CIC滤波)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 分布式算术(可重构LUT+串并混合累加)
 */
class DistributedArithmetic9 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int filterOrder = 0;
        int lutSize = 0;
        int numFilters = 0;
        double avgProcessingTimeMs = 0.0;
    };

    /** @brief Quantization bit width for input samples */
    enum Mode { Serial = 0, Parallel = 1, Hybrid = 2 };

    explicit DistributedArithmetic9(QObject *parent = nullptr);
    ~DistributedArithmetic9() override;

    /** @brief Set FIR coefficients and build LUT */
    bool setCoefficients(const QVector<double>& coeffs, int inputBits = 8);

    /** @brief Set accumulation mode */
    void setMode(Mode mode);

    /** @brief Set OSR for multi-rate (decimation factor) */
    void setOversamplingRatio(int osr);

    /** @brief Process input samples through DA FIR */
    QVector<double> process(const QVector<double>& input);

    /** @brief Get current coefficients */
    QVector<double> coefficients() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void filteringCompleted(int inputLen, int outputLen, double timeMs);

private:
    int m_inputBits = 8;
    Mode m_mode = Hybrid;
    int m_osr = 1;              // Oversampling ratio for multi-rate
    int m_taps = 0;
    int m_decimCounter = 0;

    QVector<double> m_coeffs;   // FIR coefficients
    QVector<double> m_lut;      // Precomputed LUT (2^inputBits entries)
    QVector<double> m_shiftReg; // Shift register for input history

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Build DA LUT from coefficients */
    void buildLUT();

    /** @brief Quantize a double sample to integer */
    int quantize(double sample) const;

    /** @brief Extract bit at position b from quantized value */
    static int bitAt(int value, int b);

    /** @brief Process single sample via DA (serial bit-serial) */
    double processSerial(double sample);

    /** @brief Process single sample via DA (parallel lookup) */
    double processParallel(double sample);
};
