/**
 * @file GolombCoder.h
 * @brief Golomb-Rice编码器 — 几何分布整数压缩
 *
 * 功能: 针对几何分布的整数序列进行Golomb-Rice编码/解码，
 *       自动计算最优M参数，适用于非负整数数据的无损压缩，
 *       广泛用于图像压缩和通信协议。
 *
 * 协作: HuffmanCodec(霍夫曼编码) / RunLengthCodec(游程编码)
 */
#ifndef GOLOMBCODER_H
#define GOLOMBCODER_H

#include <QObject>
#include <QVector>
#include <QByteArray>

/**
 * @brief Golomb-Rice编码器 — 几何分布整数压缩
 */
class GolombCoder : public QObject {
    Q_OBJECT

public:
    /** @brief 统计信息 */
    struct Stats {
        quint64 totalEncodes = 0;         ///< 累计编码次数
        quint64 totalDecodes = 0;         ///< 累计解码次数
        double  avgProcessingTimeMs = 0.0;///< 平均处理时间(ms)
    };

    explicit GolombCoder(QObject* parent = nullptr);

    /** @brief 编码非负整数序列
     *  @param values 非负整数数组
     *  @param m      Golomb参数(2的幂次，即Rice编码k值)
     *  @return 编码后的字节流 */
    QByteArray encode(const QVector<int>& values, int m);

    /** @brief 解码字节流为整数序列
     *  @param data  编码数据
     *  @param m     Golomb参数
     *  @param count 期望解码的元素数
     *  @return 解码后的整数数组 */
    QVector<int> decode(const QByteArray& data, int m, int count);

    /** @brief 计算最优M参数(最小化编码长度)
     *  @param values 样本数据
     *  @return 最优M参数 */
    int optimalM(const QVector<int>& values) const;

    /** @brief 获取统计 */
    const Stats& stats() const { return m_stats; }

    /** @brief 重置统计 */
    void resetStatistics();

signals:
    /** @brief 编码完成 @param encodedData 编码数据 @param compressionRatio 压缩比 */
    void encoded(const QByteArray& encodedData, double compressionRatio);

    /** @brief 解码完成 @param values 解码结果 */
    void decoded(const QVector<int>& values);

private:
    /** @brief 编码单个值(一元码+余数)
     *  @param value 非负整数
     *  @param m     Golomb参数
     *  @param bitBuffer 位缓冲
     *  @param bitPos    位位置 */
    void encodeSingle(int value, int m,
                      QByteArray& bitBuffer, int& bitPos) const;

    /** @brief 解码单个值
     *  @param data     编码数据
     *  @param bitPos   当前位位置(引用更新)
     *  @param m        Golomb参数
     *  @return 解码值 */
    int decodeSingle(const QByteArray& data, int& bitPos, int m) const;

    double m_timeSum;              ///< 处理时间累加器

    mutable Stats m_stats;         ///< 可变统计
};

#endif // GOLOMBCODER_H
