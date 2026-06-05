/**
 * @file JhHash.h
 * @brief JH 哈希计算引擎(SHA-3 候选算法简化实现)
 * @author EmbedDebug Team
 * @date 2026-06-05
 *
 * 基于 JH 算法(SHA-3 决赛候选)的简化实现。
 * 使用 1024 位内部状态和 8x8 S-box 进行消息摘要。
 */

#ifndef JHHASH_H
#define JHHASH_H

#include <QByteArray>
#include <QElapsedTimer>
#include <QObject>
#include <QtGlobal>

/**
 * @class JhHash
 * @brief JH 哈希计算器(SHA-3 决赛候选)
 *
 * 流式接口: init() -> update()* -> finalize()
 * 使用 1024 位内部状态，8x8 S-box 和线性扩散层。
 */
class JhHash : public QObject
{
    Q_OBJECT

public:
    /** @brief 统计信息结构体 */
    struct Stats {
        quint64 totalHashes = 0;       ///< 累计哈希计算次数
        quint64 totalBytesProcessed = 0; ///< 累计处理的字节总数
        quint64 totalUpdates = 0;       ///< 累计 update() 调用次数
        quint64 totalRounds = 0;        ///< 累计 S-box 置换轮次数
        double avgProcessingTimeMs = 0.0; ///< 平均每次哈希处理耗时(毫秒)
    };

    /** @brief 构造 JH 哈希计算器 @param parent 父对象 */
    explicit JhHash(QObject *parent = nullptr);

    /** @brief 初始化哈希状态 @param hashBits 输出摘要位数(224/256/384/512)，默认256 */
    void init(int hashBits = 256);

    /** @brief 追加数据到哈希计算 @param data 输入数据 */
    void update(const QByteArray &data);

    /** @brief 完成哈希计算并返回摘要 @return 哈希摘要字节数组 */
    QByteArray finalize();

    /** @brief 获取统计信息 @return 统计结构的常引用 */
    const Stats &stats() const;

    /** @brief 重置所有统计计数器 */
    void resetStatistics();

signals:
    /** @brief 哈希计算完成信号 @param hashBits 摘要位数 @param inputSize 输入数据大小 */
    void hashComputed(int hashBits, qint64 inputSize);

private:
    /** @brief 处理一个512位(64字节)消息块 @param block 消息块指针 */
    void processBlock(const quint8 *block);

    /** @brief 执行一轮 S-box 置换(对所有16个64位字) */
    void sboxLayer();

    /** @brief 执行线性扩散层(MDS矩阵混合) */
    void linearDiffusion();

    /** @brief 单个 8x8 S-box 查找 @param input 输入字节 @return 输出字节 */
    static quint8 sbox(quint8 input);

    /** @brief 应用轮常数 @param round 轮次 */
    void addRoundConstant(int round);

    /** @brief 将256位常量展开为16个64位字 @param C 常量数组的4字节表示 @param out 输出16个字 */
    static void expandConstant(const quint8 C[4], quint64 out[4]);

    quint64 m_state[16];       ///< 1024位内部状态(16个64位字)
    quint8 m_buffer[64];       ///< 64字节消息缓冲区
    int m_bufferPos;           ///< 缓冲区当前位置
    int m_hashBits;            ///< 输出摘要位数
    bool m_finalized;          ///< 是否已完成 finalize
    qint64 m_totalInputSize;   ///< 当前哈希累计输入大小

    Stats m_stats;             ///< 统计信息
    QElapsedTimer m_timer;     ///< 计时器
    double m_accumulatedTimeMs; ///< 累计处理时间(毫秒)
};

#endif // JHHASH_H
