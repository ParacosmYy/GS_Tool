/**
 * @file ReedMullerCode.h
 * @brief Reed-Muller编解码引擎 — RM(r,m)分组码
 *
 * 功能: 实现Reed-Muller编码与解码，支持可配置阶数r和参数m，
 *       生成RM(r,m)码的生成矩阵，统计编解码次数与平均耗时。
 *
 * 协作: DataChecksumVerifier(校验) / SerialFrameDecoder(帧解码)
 */
#ifndef REEDMULLERCODE_H
#define REEDMULLERCODE_H

#include <QObject>
#include <QByteArray>
#include <QVector>

/**
 * @brief Reed-Muller编解码引擎
 */
class ReedMullerCode : public QObject {
    Q_OBJECT

public:
    /** @brief 统计信息 */
    struct Stats {
        quint64 totalEncoded = 0;           ///< 累计编码次数
        quint64 totalDecoded = 0;           ///< 累计解码次数
        double  avgProcessingTimeMs = 0.0;  ///< 平均处理耗时(ms)
    };

    explicit ReedMullerCode(QObject* parent = nullptr);

    /**
     * @brief 设置RM码参数
     * @param r 阶数(0 <= r <= m)
     * @param m 参数(码长n = 2^m)
     */
    void setOrder(int r, int m);

    /**
     * @brief 编码数据
     * @param data 原始数据(比特打包为字节)
     * @return 编码后数据
     */
    QByteArray encode(const QByteArray& data);

    /**
     * @brief 解码数据(多数逻辑解码)
     * @param received 接收数据
     * @return 解码后数据
     */
    QByteArray decode(const QByteArray& received);

    /** @brief 码长 */
    int codeLength() const { return m_n; }

    /** @brief 信息位长度 */
    int infoLength() const { return m_k; }

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 解码完成信号 @param success 是否成功 */
    void decodingCompleted(bool success);

private:
    /** @brief 构建生成矩阵 */
    void buildGeneratorMatrix();

    /** @brief 计算组合数C(n,k) */
    int binomial(int n, int k) const;

    /** @brief 字节转比特数组 */
    QVector<int> bytesToBits(const QByteArray& data, int bitCount) const;

    /** @brief 比特数组转字节 */
    QByteArray bitsToBytes(const QVector<int>& bits) const;

    /** @brief 多数逻辑判决 */
    int majorityVote(const QVector<int>& received,
                     const QVector<int>& pattern) const;

    /** @brief 递归生成组合 */
    void generateCombinations(const QVector<int>& indices, int k,
                              int start, QVector<int>& current,
                              QVector<QVector<int>>& result);

    int m_r;                        ///< RM码阶数
    int m_m;                        ///< RM码参数m
    int m_n;                        ///< 码长(2^m)
    int m_k;                        ///< 信息位长度

    QVector<QVector<int>> m_genMatrix;     ///< 生成矩阵(k x n)
    double m_timeSum;                       ///< 累计耗时(ms)
    mutable Stats m_stats;                  ///< 可变统计
};

#endif // REEDMULLERCODE_H
