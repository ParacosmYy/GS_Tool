/**
 * @file Md6Hash.h
 * @brief MD6 Merkle 树哈希计算引擎(简化实现)
 * @author EmbedDebug Team
 * @date 2026-06-05
 *
 * 基于 MD6 算法的简化 Merkle 树哈希。
 * 支持可变摘要长度和可选密钥。
 */

#ifndef MD6HASH_H
#define MD6HASH_H

#include <QByteArray>
#include <QElapsedTimer>
#include <QObject>
#include <QPair>
#include <QtGlobal>

/**
 * @class Md6Hash
 * @brief MD6 Merkle 树哈希计算器
 *
 * 实现简化的 MD6 压缩函数，支持并行化的树形哈希模式。
 * 使用可配置的树层级和密钥。
 */
class Md6Hash : public QObject
{
    Q_OBJECT

public:
    /** @brief 统计信息结构体 */
    struct Stats {
        quint64 totalHashes = 0;       ///< 累计哈希计算次数
        quint64 totalBytesProcessed = 0; ///< 累计处理的字节总数
        quint64 totalCompressions = 0;  ///< 累计压缩函数调用次数
        double avgProcessingTimeMs = 0.0; ///< 平均每次哈希处理耗时(毫秒)
    };

    /** @brief 构造 MD6 哈希计算器 @param parent 父对象 */
    explicit Md6Hash(QObject *parent = nullptr);

    /** @brief 设置 Merkle 树层级数 @param levels 层级(0=串行模式, >0=并行树模式) */
    void setLevels(int levels);

    /** @brief 设置密钥(最多64字节) @param key 密钥数据 */
    void setKey(const QByteArray &key);

    /** @brief 计算数据的 MD6 哈希 @param data 输入数据 @param digestBits 摘要位数(默认256) @return 哈希摘要 */
    QByteArray hash(const QByteArray &data, int digestBits = 256);

    /** @brief 获取统计信息 @return 统计结构的常引用 */
    const Stats &stats() const;

    /** @brief 重置所有统计计数器 */
    void resetStatistics();

signals:
    /** @brief 哈希计算完成信号 @param digestBits 摘要位数 @param inputSize 输入数据大小 */
    void hashComputed(int digestBits, qint64 inputSize);

private:
    /** @brief MD6 压缩函数 @param data 输入块(128字节) @param nodeID 节点标识 @param isLast 是否最后一块 @return 压缩输出(64字节) */
    QByteArray compress(const QByteArray &data, quint64 nodeID, bool isLast);

    /** @brief 构建压缩函数的控制字 @param nodeID 节点标识 @param isLast 是否最后一块 @param level 当前层级 @return 控制字(4个quint64) */
    void buildControlWord(quint64 nodeID, bool isLast, int level,
                          quint64 ctrl[4]);

    /** @brief 将数据分块并进行 Merkle 树递归 @param data 输入数据 @param level 当前层级 @return 哈希摘要 */
    QByteArray treeHash(const QByteArray &data, int level);

    /** @brief 截断或扩展输出到指定位数 @param data 输入数据 @param bits 目标位数 @return 截断后的摘要 */
    QByteArray truncate(const QByteArray &data, int bits);

    int m_levels;               ///< Merkle 树层级数
    QByteArray m_key;           ///< 密钥(最多64字节)
    int m_chunkSize;            ///< 数据分块大小(字节)

    Stats m_stats;              ///< 统计信息
    QElapsedTimer m_timer;      ///< 计时器
    double m_accumulatedTimeMs; ///< 累计处理时间(毫秒)
};

#endif // MD6HASH_H
