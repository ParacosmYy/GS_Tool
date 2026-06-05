/**
 * @file Blake3Hash.h
 * @brief BLAKE3 哈希算法
 *
 * 实现 BLAKE3 哈希函数，支持任意长度输出、
 * 流式更新和一次性计算。基于 Bao 树模式，
 * 适用于数据完整性验证、内容寻址、Merkle树等场景。
 *
 * 协作: MerkleTree(Merkle树) / DataChecksumVerifier(校验)
 */
#ifndef BLAKE3HASH_H
#define BLAKE3HASH_H

#include <QByteArray>
#include <QElapsedTimer>
#include <QObject>
#include <QtGlobal>

/**
 * @class Blake3Hash
 * @brief BLAKE3 哈希引擎
 */
class Blake3Hash : public QObject
{
    Q_OBJECT

public:
    /** @brief 统计信息结构体 */
    struct Stats {
        quint64 totalHashes = 0;           ///< 累计哈希计算次数
        quint64 totalBytesProcessed = 0;   ///< 累计处理字节数
        quint64 totalChunks = 0;           ///< 累计处理的块数
        double avgProcessingTimeMs = 0.0;  ///< 平均处理时间(ms)
    };

    /** @brief 构造函数 @param parent 父对象 */
    explicit Blake3Hash(QObject* parent = nullptr);

    /** @brief 初始化哈希状态 */
    void init();

    /** @brief 追加数据到哈希计算 @param data 输入数据 */
    void update(const QByteArray& data);

    /**
     * @brief 完成哈希计算并返回摘要
     * @param outputBytes 输出字节数(默认32)
     * @return 哈希摘要字节串
     */
    QByteArray finalize(int outputBytes = 32);

    /**
     * @brief 一次性计算哈希(静态便捷接口)
     * @param data 输入数据
     * @param outputBytes 输出字节数(默认32)
     * @return 哈希摘要字节串
     */
    static QByteArray hash(const QByteArray& data, int outputBytes = 32);

    /** @brief 获取统计信息 @return 统计结构体的常量引用 */
    const Stats& stats() const { return m_stats; }

    /** @brief 重置所有统计计数器 */
    void resetStatistics();

signals:
    /** @brief 哈希计算完成信号 @param outputBytes 输出字节数 @param inputSize 输入数据大小 */
    void hashComputed(int outputBytes, qint64 inputSize);

private:
    /** @brief 压缩函数核心 @param chaining 8个链值 @param block 数据块 @param counter 块计数器 @param flags 标志位 @return 16个字的结果向量 */
    void compress(const quint32 chaining[8], const quint32 block[16],
                  quint64 counter, quint32 flags, quint32 out[16]);
    /** @brief G 混合函数(BLAKE2b-style) @param a,b,c,d 状态字引用 @param x,y 消息字 */
    static void g(quint32& a, quint32& b, quint32& c, quint32& d,
                  quint32 x, quint32 y);
    /** @brief 从字节数组加载16个32位字 @param src 源字节 @param offset 偏移 @param words 输出字数组 */
    static void loadWords(const QByteArray& src, int offset, quint32 words[16]);
    /** @brief 将8个32位字存储到字节串 @param words 输入字数组 @param dest 目标字节串 */
    static void storeChain(const quint32 words[8], QByteArray& dest);

    static constexpr int BLOCK_BYTES = 64;     ///< 压缩块大小(字节)
    static constexpr int CHUNK_BYTES = 1024;   ///< 块大小(字节)
    static constexpr quint32 IV_0 = 0x6A09E667; ///< BLAKE3 初始向量
    static constexpr quint32 IV_1 = 0xBB67AE85;
    static constexpr quint32 IV_2 = 0x3C6EF372;
    static constexpr quint32 IV_3 = 0xA54FF53A;
    static constexpr quint32 IV_4 = 0x510E527F;
    static constexpr quint32 IV_5 = 0x9B05688C;
    static constexpr quint32 IV_6 = 0x1F83D9AB;
    static constexpr quint32 IV_7 = 0x5BE0CD19;
    static constexpr quint32 CHUNK_START  = 1;  ///< 标志: 块开始
    static constexpr quint32 CHUNK_END    = 2;  ///< 标志: 块结束
    static constexpr quint32 PARENT       = 4;  ///< 标志: 父节点
    static constexpr quint32 ROOT         = 8;  ///< 标志: 根节点

    /** @brief 处理一个完整的 chunk(1024字节) */
    void processChunk();

    quint32 m_key[8];               ///< 链值/密钥(8个32位字)
    quint64 m_chunkCounter;         ///< 块计数器
    quint32 m_flags;                ///< 当前标志
    QByteArray m_buffer;            ///< 内部缓冲区(≤1024字节)
    int m_bufPos;                   ///< 缓冲区已用位置
    bool m_finalized;               ///< 是否已完成 finalize

    QElapsedTimer m_timer;          ///< 计时器
    Stats m_stats;                  ///< 统计信息
    double m_timeSum;               ///< 累计处理时间
};

#endif // BLAKE3HASH_H
