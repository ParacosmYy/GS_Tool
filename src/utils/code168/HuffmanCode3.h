/**
 * @file HuffmanCode3.h
 * @brief 规范霍夫曼编解码(码长限制+表序列化) — Canonical Huffman Encoder/Decoder with Code Length Limiting and Table Serialization
 *
 * 功能: 实现规范霍夫曼编码，支持码长限制(package-merge)、编解码和码表序列化，
 *       适用于嵌入式通信数据压缩与协议帧编码。
 *
 * 协作: ArithmeticCode2(算术编码) / LZ77(字典压缩) / RLE3(行程编码)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QByteArray>
#include <QMap>

/**
 * @brief 规范霍夫曼编解码器
 */
class HuffmanCode3 : public QObject {
    Q_OBJECT

public:
    /** @brief 符号类型 */
    typedef quint16 Symbol;

    /** @brief 码表条目 */
    struct CodeEntry {
        Symbol symbol = 0;
        int codeLength = 0;
        quint32 code = 0;
    };

    /** @brief 运行统计 */
    struct Stats {
        quint64 totalEncoded = 0;          ///< 累计编码符号数
        quint64 totalDecoded = 0;          ///< 累计解码符号数
        double avgProcessingTimeMs = 0.0;  ///< 平均耗时(ms)
        double compressionRatio = 0.0;     ///< 压缩比
    };

    explicit HuffmanCode3(QObject *parent = nullptr);
    ~HuffmanCode3() override;

    /** @brief 设置最大码长限制 */
    void setMaxCodeLength(int maxLen);

    /**
     * @brief 从频率表构建码表
     * @param freqs 符号频率映射
     * @return 是否成功
     */
    bool buildFromFrequencies(const QMap<Symbol, int>& freqs);

    /**
     * @brief 编码符号序列
     * @param symbols 输入符号
     * @return 编码后的字节流
     */
    QByteArray encode(const QVector<Symbol>& symbols);

    /**
     * @brief 解码字节流
     * @param data 编码数据
     * @param count 期望符号数
     * @return 解码后的符号序列
     */
    QVector<Symbol> decode(const QByteArray& data, int count);

    /** @brief 序列化码表 */
    QByteArray serializeTable() const;

    /** @brief 从序列化数据恢复码表 */
    bool deserializeTable(const QByteArray& data);

    /** @brief 获取码表 */
    QVector<CodeEntry> codeTable() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void encodingCompleted(int symbolCount, int byteCount);
    void decodingCompleted(int symbolCount);

private:
    /** @brief 构建霍夫曼树并计算码长 */
    void computeCodeLengths(const QMap<Symbol, int>& freqs);

    /** @brief Package-merge码长限制算法 */
    void limitCodeLengths(QMap<Symbol, int>& lengths, int maxLen);

    /** @brief 从码长生成规范码 */
    void buildCanonicalCodes();

    /** @brief 构建解码查找表 */
    void buildDecodeTable();

    int m_maxCodeLen = 15;

    QMap<Symbol, int> m_codeLengths;    ///< 符号->码长
    QMap<Symbol, quint32> m_codes;      ///< 符号->规范码
    QVector<CodeEntry> m_table;         ///< 码表

    /* Decode lookup: indexed by (code, length) */
    QMap<QPair<quint32, int>, Symbol> m_decodeMap;

    Stats m_stats;
    double m_timeSum = 0.0;
};
