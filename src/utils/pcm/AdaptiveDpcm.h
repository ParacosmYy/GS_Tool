/**
 * @file AdaptiveDpcm.h
 * @brief 自适应差分脉冲编码调制(ADPCM) — 编解码工具
 */
#pragma once

#include <QObject>
#include <QByteArray>
#include <QVector>

/**
 * @class AdaptiveDpcm
 * @brief 实现自适应DPCM编解码
 *
 * 使用自适应量化步长对连续信号进行差分编码。
 * 量化步长根据前一个样本的量化级别动态调整，
 * 适用于音频或传感器数据的压缩存储/传输。
 */
class AdaptiveDpcm : public QObject {
    Q_OBJECT
public:
    /** @brief 统计信息结构体 */
    struct Stats {
        quint64 totalEncoded = 0;           ///< 总编码次数
        quint64 totalDecoded = 0;           ///< 总解码次数
        double  avgProcessingTimeMs = 0.0;  ///< 平均处理时间(ms)
    };

    explicit AdaptiveDpcm(QObject* parent = nullptr);

    /**
     * @brief 对采样数据进行ADPCM编码
     * @param samples 原始采样数据
     * @return 编码后的字节数据
     *
     * 编码格式: [quantLevels(uint8)] [sample0(double,8B)] [deltaCode...]
     * 每个deltaCode为ceil(log2(quantLevels))位，字节对齐打包
     */
    QByteArray encode(const QVector<double>& samples);

    /**
     * @brief 对ADPCM数据进行解码
     * @param data 编码后的字节数据
     * @return 解码后的采样数据
     */
    QVector<double> decode(const QByteArray& data);

    /**
     * @brief 设置量化级数
     * @param levels 量化级数 (2~256，默认16)
     */
    void setQuantLevels(int levels);

    /** @brief 获取当前量化级数 */
    int quantLevels() const { return m_quantLevels; }

    /** @brief 获取统计信息 */
    const Stats& stats() const { return m_stats; }

    /** @brief 重置统计信息 */
    void resetStatistics();

signals:
    /** @brief 编码完成信号 @param originalSize 原始大小 @param encodedSize 编码大小 */
    void encodingCompleted(int originalSize, int encodedSize);

private:
    Stats  m_stats;
    double m_timeSum = 0.0;       ///< 累计处理时间

    int    m_quantLevels = 16;    ///< 量化级数
    double m_stepSize     = 1.0;  ///< 当前自适应步长
    double m_minStep      = 0.001; ///< 最小步长
    double m_maxStep      = 1000.0; ///< 最大步长

    static constexpr double STEP_SCALE_UP   = 1.2;   ///< 步长放大系数
    static constexpr double STEP_SCALE_DOWN = 0.9;   ///< 步长缩小系数

    /**
     * @brief 将差值量化为编码索引
     * @param delta 差值
     * @return 量化索引 (非负)
     */
    int quantize(double delta);

    /**
     * @brief 将量化索引反量化为差值
     * @param code 量化索引
     * @return 反量化差值
     */
    double dequantize(int code);

    /**
     * @brief 根据量化级别更新步长
     * @param code 当前量化码
     */
    void updateStepSize(int code);

    /** @brief 写位到字节缓冲 */
    void writeBits(QByteArray& buf, int value, int bitCount, int& bitPos) const;

    /** @brief 从字节缓冲读位 */
    int readBits(const QByteArray& buf, int bitCount, int& bitPos) const;
};
