/**
 * @file HuffmanCodec.h
 * @brief 霍夫曼编解码器 — 频率驱动的最优前缀编码
 *
 * 提供基于频率表的霍夫曼树构建、位流编码与解码,
 * 适用于嵌入式调试场景中的数据压缩和变长编码应用。
 */
#ifndef HUFFMAN_CODEC_H
#define HUFFMAN_CODEC_H

#include <QObject>
#include <QByteArray>
#include <QMap>
#include <QVector>

/**
 * @class HuffmanCodec
 * @brief 霍夫曼编解码器 — 频率统计 + 前缀树 + 位流处理
 *
 * 典型用法:
 * @code
 *   HuffmanCodec codec;
 *   QByteArray encoded = codec.encode(rawData);
 *   QByteArray decoded = codec.decode(encoded);
 * @endcode
 */
class HuffmanCodec : public QObject {
    Q_OBJECT

public:
    /** @brief 操作统计结构 */
    struct Stats {
        quint64 totalEncodes = 0;            ///< 编码操作总次数
        quint64 totalDecodes = 0;            ///< 解码操作总次数
        double  avgCompressionRatio = 0.0;   ///< 平均压缩比(输出/输入)
    };

    /** @brief 构造函数 @param parent 父对象 */
    explicit HuffmanCodec(QObject* parent = nullptr);

    /** @brief 析构函数 */
    ~HuffmanCodec() override;

    // ── 编解码 ──

    /**
     * @brief 霍夫曼编码
     * @param data 原始数据
     * @return 编码数据(含头部: 频率表 + 位流); 失败返回空
     */
    QByteArray encode(const QByteArray& data);

    /**
     * @brief 霍夫曼解码
     * @param data 编码数据(须含频率表头部)
     * @return 解码后原始数据; 失败返回空并发射error信号
     */
    QByteArray decode(const QByteArray& data);

    // ── 统计 ──

    /** @brief 获取当前统计数据快照 */
    Stats stats() const;

    /** @brief 重置所有统计计数器归零 */
    void resetStatistics();

signals:
    /** @brief 编码完成信号 @param result 编码结果 @param ratio 压缩比 */
    void encoded(const QByteArray& result, double ratio);
    /** @brief 解码完成信号 @param result 解码结果 */
    void decoded(const QByteArray& result);
    /** @brief 错误信号 @param errorMessage 错误描述 */
    void error(const QString& errorMessage);

private:
    /** @brief 霍夫曼树节点 */
    struct HuffNode {
        int       symbol = -1;        ///< 字节值(-1=内部节点)
        quint32   freq = 0;           ///< 频率
        HuffNode* left = nullptr;     ///< 左子节点(编码0)
        HuffNode* right = nullptr;    ///< 右子节点(编码1)
    };

    /** @brief 构建频率表 @param data 输入数据 @return 256元素频率数组 */
    QVector<quint32> buildFreqTable(const QByteArray& data) const;

    /** @brief 构建霍夫曼树 @param freq 频率表 @return 根节点 */
    HuffNode* buildTree(const QVector<quint32>& freq);

    /** @brief 递归生成编码表 @param node 当前节点 @param code 当前编码位串 */
    void generateCodes(HuffNode* node, const QByteArray& code);

    /** @brief 递归释放霍夫曼树 @param node 当前节点 */
    void freeTree(HuffNode* node);

    /** @brief 序列化频率表到字节数组 */
    QByteArray serializeFreqTable(const QVector<quint32>& freq) const;

    /** @brief 从字节数组反序列化频率表 */
    QVector<quint32> deserializeFreqTable(const QByteArray& data,
                                          int* bytesRead) const;

    /** @brief 位串('0'/'1'序列)转打包字节数组 */
    QByteArray bitsToPackedBytes(const QByteArray& bits) const;

    /** @brief 打包字节数组转位串 */
    QByteArray packedBytesToBits(const QByteArray& bytes,
                                 int totalBits) const;

    /** @brief 更新平均压缩比 */
    void updateAvgRatio(double ratio);

    /** @brief 编码表: 字节值→编码位串('0'/'1'序列) */
    QMap<int, QByteArray> m_codeTable;

    /** @brief 操作统计 */
    Stats m_stats;
};

#endif // HUFFMAN_CODEC_H
