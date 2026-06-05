/**
 * @file GolayCode.h
 * @brief 扩展Golay码 (24,12) — 纠错编码
 *
 * 功能: 实现扩展Golay码(24,12,8)的编码与解码，
 *       支持最多3位错误纠正，统计编码/解码次数/耗时。
 */
#pragma once

#include <QObject>
#include <QByteArray>

class GolayCode : public QObject {
    Q_OBJECT
public:
    /** 统计信息 */
    struct Stats {
        quint64 totalEncoded = 0;        ///< 总编码次数
        quint64 totalDecoded = 0;        ///< 总解码次数
        double  avgProcessingTimeMs = 0.0; ///< 平均处理时间(ms)
    };

    explicit GolayCode(QObject* parent = nullptr);

    /**
     * @brief 编码数据(扩展Golay码 24,12)
     * @param data 输入数据(每12bit扩展为24bit)
     * @return 编码后的数据
     */
    QByteArray encode(const QByteArray& data);

    /**
     * @brief 解码接收到的数据(纠正最多3位错误)
     * @param received 接收到的编码数据
     * @return 解码后的原始数据
     */
    QByteArray decode(const QByteArray& received);

    /**
     * @brief 计算码字的汉明重量
     * @param codeword 码字
     * @return 汉明重量(1的个数)
     */
    int hammingWeight(const QByteArray& codeword);

    /** @brief 获取统计信息 */
    const Stats& stats() const { return m_stats; }

    /** @brief 重置统计 */
    void resetStatistics();

signals:
    /** @brief 解码完成信号 @param corrected 是否进行了纠错 */
    void decodingCompleted(bool corrected);

private:
    /** @brief 初始化生成矩阵 */
    void initGeneratorMatrix();

    /** @brief 将12bit数据编码为24bit码字 */
    quint32 encode12Bit(quint32 data) const;

    /** @brief 解码24bit码字返回12bit数据 */
    quint32 decode24Bit(quint32 codeword, bool& corrected);

    /** @brief 计算伴随式 */
    quint32 syndrome(quint32 codeword) const;

    /** @brief 计算整数中1的位数 */
    int popcount(quint32 x) const;

    Stats  m_stats;
    double m_timeSum;
    quint32 m_generatorMatrix[12]; ///< Golay码生成矩阵行
};
