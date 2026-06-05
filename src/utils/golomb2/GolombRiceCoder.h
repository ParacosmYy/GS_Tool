/**
 * @file GolombRiceCoder.h
 * @brief Golomb-Rice编解码器(Golomb-Rice Coder)
 */

#pragma once

#include <QObject>
#include <QByteArray>
#include <QVector>

/**
 * @class GolombRiceCoder
 * @brief Golomb-Rice编解码器 — 效率最优的前缀编码
 *
 * 支持自适应参数选择、批量编解码、比特级操作。
 * 适用于非负整数压缩、音频编码、图像无损压缩等场景。
 */
class GolombRiceCoder : public QObject
{
    Q_OBJECT

public:
    /** @brief 统计信息 */
    struct Stats {
        int totalEncoded = 0;       /**< 总编码次数 */
        int totalDecoded = 0;       /**< 总解码次数 */
        long long totalBitsIn = 0;  /**< 输入总比特数 */
        long long totalBitsOut = 0; /**< 输出总比特数 */
        double avgProcessingTimeMs = 0.0; /**< 平均处理时间(ms) */
    };

    /** @brief 构造函数 */
    explicit GolombRiceCoder(int m = 4, QObject* parent = nullptr);

    /**
     * @brief 编码单个非负整数
     * @param value 待编码值
     * @return 编码后的字节序列(高位对齐)
     */
    QByteArray encode(quint32 value);

    /**
     * @brief 编码非负整数向量
     * @param values 待编码值列表
     * @return 编码后的字节序列
     */
    QByteArray encodeBatch(const QVector<quint32>& values);

    /**
     * @brief 解码字节序列
     * @param data 编码数据
     * @param count 期望解码的整数个数
     * @return 解码后的整数列表
     */
    QVector<quint32> decode(const QByteArray& data, int count);

    /**
     * @brief 计算最优M参数(基于几何分布假设)
     * @param values 样本数据
     * @return 推荐的M值
     */
    static int optimalM(const QVector<quint32>& values);

    /**
     * @brief 设置Rice参数M(必须是2的幂)
     * @param m 新的M值
     */
    void setM(int m);

    /** @brief 获取当前M值 */
    int m() const;

    /** @brief 计算编码后比特数(不实际编码) */
    int encodedBits(quint32 value) const;

    /** @brief 获取统计 */
    Stats stats() const;

    /** @brief 重置统计 */
    void resetStatistics();

signals:
    /** @brief 编码完成信号 */
    void encodingCompleted(int count, int originalBits, int encodedBits);

private:
    int m_m;            /**< Rice参数M(2^k) */
    int m_k;            /**< log2(M) */
    Stats m_stats;
    double m_timeSum;
};
