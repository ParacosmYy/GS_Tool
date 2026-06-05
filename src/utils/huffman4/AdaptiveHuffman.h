/**
 * @file AdaptiveHuffman.h
 * @brief 自适应Huffman编码
 */

#pragma once

#include <QObject>
#include <QByteArray>
#include <QMap>

/**
 * @brief 自适应Huffman编码器(FGK算法)
 *
 * 动态构建Huffman树,无需预先统计频率。
 * 支持单遍编码/解码,适用于流式数据。
 */
class AdaptiveHuffman : public QObject
{
    Q_OBJECT

public:
    /** @brief 统计信息 */
    struct Stats {
        int totalEncoded = 0;             ///< 编码次数
        int totalDecoded = 0;             ///< 解码次数
        int totalBytesIn = 0;             ///< 输入字节数
        int totalBytesOut = 0;            ///< 输出字节数
        double avgProcessingTimeMs = 0.0; ///< 平均处理时间(ms)
    };

    explicit AdaptiveHuffman(int alphabetSize = 256, QObject* parent = nullptr);

    /**
     * @brief 编码数据
     * @param data 输入数据
     * @return 编码后的比特流(QByteArray低位在前)
     */
    QByteArray encode(const QByteArray& data);

    /**
     * @brief 解码数据
     * @param data 编码后的数据
     * @param originalSize 原始数据大小
     * @return 解码后的数据
     */
    QByteArray decode(const QByteArray& data, int originalSize);

    /**
     * @brief 获取当前压缩率
     */
    double compressionRatio() const;

    Stats stats() const;
    void resetStatistics();

signals:
    /** @brief 编码完成信号 */
    void encodingCompleted(int bytesIn, int bytesOut);

private:
    /** @brief Huffman树节点 */
    struct Node {
        int symbol;        ///< 符号(或-1表示NYT/内部节点)
        int weight;        ///< 权重(频率)
        int order;         ///< 节点序号(用于兄弟性质)
        Node* parent;
        Node* left;
        Node* right;
    };

    int m_alphabetSize;
    Node* m_root;
    Node* m_nyt;                     ///< NYT(未传输)节点
    QMap<int, Node*> m_symbolNodes;  ///< 符号到节点映射
    Stats m_stats;
    double m_timeSum = 0.0;

    Node* createNode(int symbol, int weight, int order, Node* parent);
    void updateTree(Node* node);
    Node* findLeader(Node* node);
    void swapNodes(Node* a, Node* b);
    void deleteTree(Node* node);
    void resetTree();

    /* 比特流写入 */
    QByteArray m_bitBuffer;
    int m_bitPos;
    void writeBit(int bit);
    void writeCode(Node* node);

    /* 比特流读取 */
    int m_readBitPos;
    int readBit(const QByteArray& data);
};
