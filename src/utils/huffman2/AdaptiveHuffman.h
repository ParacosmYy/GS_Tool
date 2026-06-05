/**
 * @file AdaptiveHuffman.h
 * @brief 自适应霍夫曼编码器 — FGK算法, 动态树更新
 *
 * 实现FGK(Faller-Gallager-Knuth)自适应霍夫曼编码, 无需预先
 * 构建频率表, 单遍扫描即可完成编码/解码。适用于实时数据流
 * 和嵌入式调试中的自适应压缩场景。
 */
#ifndef ADAPTIVE_HUFFMAN_H
#define ADAPTIVE_HUFFMAN_H

#include <QObject>
#include <QByteArray>
#include <QMap>
#include <QVector>

/**
 * @class AdaptiveHuffman
 * @brief 自适应霍夫曼编码器(FGK算法)
 *
 * 单遍扫描编码/解码, 动态更新霍夫曼树。
 * 支持任意字节符号(0~255)的自适应频率统计和编码。
 */
class AdaptiveHuffman : public QObject {
    Q_OBJECT

public:
    /** @brief 操作统计结构 */
    struct Stats {
        quint64 totalEncodes = 0;       ///< 编码操作总次数
        quint64 totalDecodes = 0;       ///< 解码操作总次数
        quint64 totalBitsOut = 0;       ///< 输出比特总数
        double  avgBitsPerSymbol = 0.0; ///< 平均每符号比特数
        int     treeNodes = 0;          ///< 当前树节点数
    };

    /** @brief 构造函数 @param parent 父对象 */
    explicit AdaptiveHuffman(QObject* parent = nullptr);

    /** @brief 析构函数 */
    ~AdaptiveHuffman() override;

    // ── 编解码 ──

    /**
     * @brief 自适应霍夫曼编码
     * @param data 原始数据
     * @return 编码后比特流(打包为字节数组)
     */
    QByteArray encode(const QByteArray& data);

    /**
     * @brief 自适应霍夫曼解码
     * @param data 编码数据
     * @param originalSize 原始数据长度
     * @return 解码后原始数据
     */
    QByteArray decode(const QByteArray& data, int originalSize);

    // ── 统计 ──

    /** @brief 获取当前统计数据快照 */
    Stats stats() const;

    /** @brief 重置所有统计计数器归零 */
    void resetStatistics();

signals:
    /** @brief 编码完成信号 @param result 编码结果 @param bitsPerSym 每符号比特数 */
    void encoded(const QByteArray& result, double bitsPerSym);
    /** @brief 解码完成信号 @param result 解码结果 */
    void decoded(const QByteArray& result);
    /** @brief 错误信号 @param msg 错误描述 */
    void error(const QString& msg);

private:
    /** @brief 树节点结构 */
    struct Node {
        int symbol = -1;    ///< 符号(-1为NYT, -2为内部节点)
        int weight = 0;     ///< 权重(频率)
        int order = 0;      ///< 节点序号(用于兄弟性质)
        int parent = -1;    ///< 父节点索引
        int left = -1;      ///< 左子节点索引
        int right = -1;     ///< 右子节点索引
    };

    /** @brief 初始化树(NYT节点) */
    void initTree();

    /** @brief 查找符号对应的叶节点 @return 节点索引, -1表示未找到 */
    int findSymbol(int sym) const;

    /** @brief 查找同权重最高序号节点(用于交换) */
    int findLeader(int nodeIdx) const;

    /** @brief 交换两个节点 */
    void swapNodes(int a, int b);

    /** @brief 更新树: 插入或增加符号频率 */
    void updateTree(int nodeIdx);

    /** @brief 获取节点的编码路径(比特序列) */
    QVector<bool> getCode(int nodeIdx) const;

    /** @brief 将比特序列打包为字节数组 */
    QByteArray packBits(const QVector<bool>& bits) const;

    /** @brief 将字节数组解包为比特序列 */
    QVector<bool> unpackBits(const QByteArray& data, int bitCount) const;

    QVector<Node> m_tree;          ///< 霍夫曼树节点数组
    QMap<int, int> m_symbolMap;    ///< 符号→节点索引映射
    int m_nytIndex = 0;            ///< NYT(Not Yet Transmitted)节点索引
    int m_nextOrder = 0;           ///< 下一个节点序号

    mutable Stats m_stats;         ///< 操作统计
};

#endif // ADAPTIVE_HUFFMAN_H
