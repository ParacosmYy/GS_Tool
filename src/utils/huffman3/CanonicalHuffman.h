/**
 * @file CanonicalHuffman.h
 * @brief 规范霍夫曼编码 — 基于符号频率的最优前缀码
 *
 * 功能: 根据符号频率构建规范霍夫曼码表，支持编码和解码。
 *       规范形式仅需存储码长即可重建码表。
 *
 * 协作: HuffmanCodec(标准霍夫曼) / ShannonCoder(香农编码)
 */
#ifndef CANONICALHUFFMAN_H
#define CANONICALHUFFMAN_H

#include <QObject>
#include <QVector>
#include <QMap>
#include <QByteArray>

/**
 * @brief 规范霍夫曼编解码器
 */
class CanonicalHuffman : public QObject {
    Q_OBJECT

public:
    /** @brief 统计信息 */
    struct Stats {
        quint64 totalEncodes = 0;        ///< 累计编码次数
        quint64 totalDecodes = 0;        ///< 累计解码次数
        double  avgProcessingTimeMs = 0.0; ///< 平均处理时间(ms)
    };

    explicit CanonicalHuffman(QObject* parent = nullptr);

    /** @brief 根据频率构建码表
     *  @param frequencies 符号→频率映射 */
    void buildCodeTable(const QMap<int, double>& frequencies);

    /** @brief 编码数据
     *  @param data 待编码数据(符号列表)
     *  @return 编码后的比特流(每字节8bit) */
    QByteArray encode(const QVector<int>& data);

    /** @brief 解码数据
     *  @param encoded 编码后的比特流
     *  @param symbolCount 原始符号数量
     *  @return 解码后的符号列表 */
    QVector<int> decode(const QByteArray& encoded, int symbolCount);

    /** @brief 获取符号的码字 @param symbol 符号 @return 码字(比特串) */
    QByteArray codeWord(int symbol) const;

    /** @brief 获取统计 */
    const Stats& stats() const { return m_stats; }

    /** @brief 重置统计 */
    void resetStatistics();

signals:
    /** @brief 编码完成 @param inputSize 输入符号数 @param outputBits 输出比特数 */
    void encodeCompleted(int inputSize, int outputBits);

    /** @brief 解码完成 @param symbolCount 解码符号数 */
    void decodeCompleted(int symbolCount);

private:
    /** @brief 从码长生成规范码 */
    void buildCanonicalCodes();

    QMap<int, int> m_codeLengths;      ///< 符号→码长映射
    QMap<int, QByteArray> m_codeWords; ///< 符号→码字映射
    QMap<QByteArray, int> m_decodeMap; ///< 码字→符号映射
    double m_timeSum;                   ///< 处理时间累加器
    Stats  m_stats;                     ///< 统计信息
};

#endif // CANONICALHUFFMAN_H
