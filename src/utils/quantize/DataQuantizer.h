/**
 * @file DataQuantizer.h
 * @brief 数据量化引擎 — 均匀/对数/μ律/A律量化
 *
 * 功能: 4种量化方法，支持可配置位深和范围，
 *       提供量化和反量化操作，计算SNR。
 *
 * 协作: DataCompressor(压缩预处理) / DataNormalizer(归一化后量化)
 */
#ifndef DATAQUANTIZER_H
#define DATAQUANTIZER_H

#include <QObject>
#include <QVector>

class DataQuantizer : public QObject {
    Q_OBJECT

public:
    /** @brief 量化方法 */
    enum class QuantizeMethod {
        Uniform,        ///< 均匀量化
        Logarithmic,    ///< 对数量化
        MuLaw,          ///< μ律压扩(μ=255)
        ALaw            ///< A律压扩(A=87.6)
    };
    Q_ENUM(QuantizeMethod)

    /** @brief 量化配置 */
    struct QuantizeConfig {
        int bits = 8;               ///< 量化位深
        double minVal = -1.0;       ///< 最小值
        double maxVal = 1.0;        ///< 最大值
    };

    /** @brief 统计 */
    struct Stats {
        quint64 totalValuesQuantized = 0;   ///< 累计量化值数
        quint64 totalBatchesProcessed = 0;  ///< 累计批处理数
        double  peakSnr = 0.0;              ///< 峰值信噪比(dB)
        double  averageError = 0.0;         ///< 平均量化误差
        double  mse = 0.0;                  ///< 均方误差
    };

    explicit DataQuantizer(QObject* parent = nullptr);

    void setMethod(QuantizeMethod method);
    void setConfig(const QuantizeConfig& config);

    /** @brief 量化单个值 @param value 输入 @return 量化编码 */
    int quantize(double value);

    /** @brief 反量化 @param code 编码 @return 重建值 */
    double dequantize(int code) const;

    /** @brief 批量量化并重建 @param data 数据 @return 量化后数据 */
    QVector<double> quantizeBatch(const QVector<double>& data);

    /** @brief 计算SNR @param original 原始 @param quantized 量化 @return SNR(dB) */
    double getSnr(const QVector<double>& original,
                  const QVector<double>& quantized) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void quantized(double original, int code, double reconstructed);

private:
    int quantizeUniform(double value) const;
    int quantizeLog(double value) const;
    int quantizeMuLaw(double value) const;
    int quantizeALaw(double value) const;

    double dequantizeUniform(int code) const;
    double dequantizeLog(int code) const;
    double dequantizeMuLaw(int code) const;
    double dequantizeALaw(int code) const;

    QuantizeMethod m_method;
    QuantizeConfig m_config;
    int m_levels;

    double m_errorSum;
    double m_mseSum;
    Stats m_stats;
};

#endif // DATAQUANTIZER_H
