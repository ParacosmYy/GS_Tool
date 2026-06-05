/**
 * @file HammingCode.h
 * @brief 汉明纠错码(SECDED) — 编码/解码/纠错
 *
 * 功能: 实现汉明(SECDED)码，支持可变数据位宽，自动计算校验位数量，
 *       提供编码、解码、校验子计算、错误检测与单比特纠错。
 *
 * 协作: DataValidator(数据校验) / CrcStreamVerifier(流校验)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 汉明码编解码器(SECDED)
 */
class HammingCode : public QObject {
    Q_OBJECT

public:
    /** @brief 统计信息 */
    struct Stats {
        quint64 totalEncoded = 0;          ///< 累计编码次数
        quint64 totalDecoded = 0;          ///< 累计解码次数
        quint64 totalCorrected = 0;        ///< 累计纠错次数
        double  avgProcessingTimeMs = 0.0; ///< 平均处理耗时(ms)
    };

    /**
     * @brief 构造函数
     * @param dataBits 数据位数(默认4)
     * @param parent 父对象
     */
    explicit HammingCode(int dataBits = 4, QObject* parent = nullptr);

    /**
     * @brief 编码(添加校验位+总校验位)
     * @param data 数据位(仅0/1)
     * @return 编码后码字(含总校验位)
     */
    QVector<int> encode(const QVector<int>& data);

    /**
     * @brief 解码(纠错后提取数据位)
     * @param received 接收到的码字
     * @return 纠正后的数据位
     */
    QVector<int> decode(const QVector<int>& received);

    /**
     * @brief 计算校验子
     * @param received 接收码字
     * @return 校验子值(0=无错误)
     */
    int syndrome(const QVector<int>& received);

    /**
     * @brief 检测是否存在错误
     * @param received 接收码字
     * @return 是否有错误
     */
    bool hasError(const QVector<int>& received);

    /**
     * @brief 纠正单比特错误
     * @param received 接收码字(会被就地修改)
     * @return 是否纠正成功
     */
    bool correctError(QVector<int>& received);

    /** @brief 获取数据位数 @return 数据位数 */
    int dataBits() const { return m_dataBits; }

    /** @brief 获取总码字长度(含总校验位) @return 码字长度 */
    int totalBits() const { return m_dataBits + m_parityBits + 1; }

    /** @brief 获取统计 @return 统计信息常引用 */
    const Stats& stats() const { return m_stats; }

    /** @brief 重置统计 */
    void resetStatistics();

signals:
    /** @brief 编码完成 @param codeLen 码字长度 */
    void encoded(int codeLen);

    /** @brief 错误纠正 @param position 错误位置 */
    void errorCorrected(int position);

private:
    /**
     * @brief 计算需要的校验位数
     * @param dataBits 数据位数
     * @return 校验位数
     */
    int computeParityBits(int dataBits) const;

    /**
     * @brief 计算整体校验位(偶校验)
     * @param codeword 码字(不含总校验位)
     * @return 总校验位值
     */
    int computeOverallParity(const QVector<int>& codeword) const;

    int m_dataBits;       ///< 数据位数
    int m_parityBits;     ///< 校验位数
    Stats m_stats;        ///< 统计信息
    double m_timeSum;     ///< 累计耗时
};
