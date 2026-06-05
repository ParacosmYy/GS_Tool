/**
 * @file ArithmeticCoder.h
 * @brief 算术编码压缩器 — 基于频率表的高效熵编码
 *
 * 功能: 实现算术编码压缩与解压，支持自适应频率表更新、
 *       多符号字母表、编码效率统计，适用于通信数据压缩、
 *       协议载荷优化等场景。压缩率接近Shannon熵极限。
 *
 * 协作: DataCompressor(数据压缩) / EntropyCalculator(熵分析)
 */
#pragma once

#include <QObject>
#include <QByteArray>
#include <QMap>
#include <QVector>

/**
 * @brief 算术编码压缩/解压引擎
 *
 * 典型用法:
 * @code
 *   ArithmeticCoder coder;
 *   coder.buildFrequencyTable(sampleData);
 *   QByteArray compressed = coder.compress(data);
 *   QByteArray recovered = coder.decompress(compressed);
 * @endcode
 */
class ArithmeticCoder : public QObject {
    Q_OBJECT

public:
    /** @brief 编码结果 */
    struct EncodeResult {
        QByteArray encoded;                ///< 编码后数据
        int originalSize = 0;              ///< 原始大小
        int encodedSize = 0;               ///< 编码后大小
        double compressionRatio = 0.0;     ///< 压缩比
        double entropy = 0.0;              ///< 数据信息熵
        double elapsedMs = 0.0;           ///< 编码耗时
    };

    /** @brief 频率表条目 */
    struct SymbolFreq {
        quint8 symbol = 0;            ///< 符号值
        quint64 count = 0;            ///< 出现次数
        double cumulativeProb = 0.0;  ///< 累积概率
    };

    /** @brief 统计数据 */
    struct Stats {
        int totalEncodes = 0;                  ///< 累计编码次数
        int totalDecodes = 0;                  ///< 累计解码次数
        double avgProcessingTimeMs = 0.0;      ///< 平均处理时间(ms)
        int totalEncodeErrors = 0;             ///< 编码错误次数
        qint64 totalBytesEncoded = 0;          ///< 累计编码字节数
    };

    explicit ArithmeticCoder(QObject* parent = nullptr);

    /**
     * @brief 从样本数据构建频率表
     * @param sample 用于统计的样本数据
     */
    void buildFrequencyTable(const QByteArray& sample);

    /**
     * @brief 手动设置符号频率
     * @param frequencies 符号->频率映射
     */
    void setFrequencyTable(const QMap<quint8, quint64>& frequencies);

    /**
     * @brief 压缩数据
     * @param data 原始数据
     * @return 编码结果
     */
    EncodeResult compress(const QByteArray& data);

    /**
     * @brief 解压数据
     * @param data 编码数据(须含频率表头部)
     * @return 解码后数据; 失败返回空
     */
    QByteArray decompress(const QByteArray& data);

    /**
     * @brief 预估压缩比(基于当前频率表和输入数据)
     * @param data 待分析数据
     * @return 预估压缩比(0.0~1.0)
     */
    double estimateRatio(const QByteArray& data) const;

    /**
     * @brief 获取当前频率表
     * @return 符号频率列表(按累积概率排序)
     */
    QVector<SymbolFreq> frequencyTable() const;

    /** @brief 获取统计 @return 统计数据 */
    Stats stats() const { return m_stats; }

    /** @brief 重置统计 */
    void resetStatistics();

signals:
    /** @brief 编码完成 @param result 编码结果 */
    void encodeCompleted(const EncodeResult& result);

    /** @brief 解码完成 @param size 解码数据大小 */
    void decodeCompleted(int size);

    /** @brief 错误信号 @param msg 错误描述 */
    void error(const QString& msg);

private:
    /** @brief 序列化频率表到字节数组 */
    QByteArray serializeFreqTable() const;

    /** @brief 从字节数组反序列化频率表 */
    int deserializeFreqTable(const QByteArray& data, int offset);

    /** @brief 计算信息熵 */
    double computeEntropy() const;

    /** @brief 计算理论编码长度 */
    double theoreticalLength(int dataLen) const;

    /** @brief 查找符号对应的累积区间 */
    QPair<double, double> symbolRange(quint8 symbol) const;

    static constexpr int kFreqHeaderMagic = 0xAC;   ///< 频率表头部魔数
    static constexpr double kPrecision = 1.0 / (1ULL << 32); ///< 编码精度

    QMap<quint8, quint64> m_freq;           ///< 符号频率
    QVector<SymbolFreq> m_cumulative;       ///< 累积概率表(排序后)
    quint64 m_totalSymbols = 0;             ///< 总符号数
    Stats m_stats;                          ///< 统计数据
    double m_timeSum = 0.0;                ///< 时间累加器
};
