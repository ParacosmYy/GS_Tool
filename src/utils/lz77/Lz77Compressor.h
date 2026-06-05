/**
 * @file Lz77Compressor.h
 * @brief LZ77滑动窗口压缩 — 经典字典压缩算法
 *
 * 功能: 基于滑动窗口的LZ77压缩/解压缩，使用(offset, length, literal)三元组。
 *       适用于流式数据的实时压缩。
 *
 * 协作: HuffmanCodec(熵编码) / LzwCompressor(字典压缩)
 */
#ifndef LZ77COMPRESSOR_H
#define LZ77COMPRESSOR_H

#include <QObject>
#include <QByteArray>
#include <QVector>

/**
 * @brief LZ77滑动窗口压缩器
 */
class Lz77Compressor : public QObject {
    Q_OBJECT

public:
    /** @brief LZ77三元组 */
    struct Token {
        quint16 offset = 0;   ///< 回溯偏移(0=无匹配)
        quint16 length = 0;   ///< 匹配长度
        quint8  literal = 0;  ///< 字面量字节
    };

    /** @brief 统计信息 */
    struct Stats {
        quint64 totalCompressions = 0;   ///< 累计压缩次数
        quint64 totalDecompressions = 0; ///< 累计解压次数
        double  avgProcessingTimeMs = 0.0; ///< 平均处理时间(ms)
    };

    explicit Lz77Compressor(QObject* parent = nullptr);

    /** @brief 压缩数据
     *  @param data 输入数据
     *  @return 压缩后的token序列 */
    QVector<Token> compress(const QByteArray& data);

    /** @brief 解压数据
     *  @param tokens 压缩token序列
     *  @return 解压后的数据 */
    QByteArray decompress(const QVector<Token>& tokens);

    /** @brief 设置滑动窗口大小 @param size 窗口大小 */
    void setWindowSize(int size);

    /** @brief 设置最大匹配长度 @param len 最大匹配长度 */
    void setMaxMatchLength(int len);

    /** @brief 获取统计 */
    const Stats& stats() const { return m_stats; }

    /** @brief 重置统计 */
    void resetStatistics();

signals:
    /** @brief 压缩完成 @param inputSize 输入大小 @param outputSize 输出大小 */
    void compressionCompleted(int inputSize, int outputSize);

    /** @brief 解压完成 @param outputSize 解压后大小 */
    void decompressionCompleted(int outputSize);

private:
    /** @brief 在窗口中搜索最长匹配
     *  @param data 数据 @param pos 当前位置 @param offset 输出偏移 @param length 输出长度 */
    void findLongestMatch(const QByteArray& data, int pos,
                          quint16& offset, quint16& length) const;

    int m_windowSize;      ///< 滑动窗口大小
    int m_maxMatchLength;  ///< 最大匹配长度
    double m_timeSum;      ///< 处理时间累加器
    Stats  m_stats;        ///< 统计信息
};

#endif // LZ77COMPRESSOR_H
