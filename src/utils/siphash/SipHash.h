/**
 * @file SipHash.h
 * @brief SipHash-2-4 快速 MAC/哈希函数
 *
 * 实现 SipHash-2-4 和 SipHash-1-3 (HalfSipHash) 算法，
 * 提供 64 位哈希输出。适用于哈希表防碰撞、
 * 消息认证码(MAC)等场景。
 *
 * 协作: ChecksumCalculator(校验) / KeccakHash(加密哈希)
 */
#ifndef SIPHASH_H
#define SIPHASH_H

#include <QByteArray>
#include <QElapsedTimer>
#include <QObject>
#include <QtGlobal>

/**
 * @class SipHash
 * @brief SipHash-2-4 快速 MAC/哈希引擎
 */
class SipHash : public QObject
{
    Q_OBJECT

public:
    /** @brief 统计信息结构体 */
    struct Stats {
        quint64 totalHashes = 0;           ///< 累计哈希计算次数
        quint64 totalBytesProcessed = 0;   ///< 累计处理字节数
        double avgProcessingTimeMs = 0.0;  ///< 平均处理时间(ms)
    };

    /** @brief 构造函数 @param parent 父对象 */
    explicit SipHash(QObject* parent = nullptr);

    /** @brief 设置密钥 @param k0 密钥低64位 @param k1 密钥高64位 */
    void setKey(quint64 k0, quint64 k1);

    /**
     * @brief 计算 SipHash-2-4 哈希值
     * @param data 输入数据
     * @return 64 位哈希值
     */
    quint64 compute(const QByteArray& data);

    /**
     * @brief 计算 SipHash-1-3 (HalfSipHash) 哈希值
     * @param data 输入数据
     * @return 64 位哈希值
     */
    quint64 computeHalf(const QByteArray& data);

    /** @brief 获取统计信息 @return 统计结构体的常量引用 */
    const Stats& stats() const { return m_stats; }

    /** @brief 重置所有统计计数器 */
    void resetStatistics();

signals:
    /** @brief 哈希计算完成信号 @param inputSize 输入数据大小 */
    void hashComputed(qint64 inputSize);

private:
    /** @brief SipRound 宏内联函数(2轮压缩) @param v0-v3 状态字引用 */
    static inline void sipRound(quint64& v0, quint64& v1,
                                quint64& v2, quint64& v3);
    /** @brief 从字节串读取小端64位整数 @param data 数据 @param offset 偏移 */
    static quint64 readLE64(const QByteArray& data, int offset);
    /** @brief 左旋 @param x 值 @param b 位数 */
    static constexpr quint64 rotl(quint64 x, int b);

    /** @brief 通用 SipHash 计算 @param data 输入数据 @param cRound 压缩轮数 @param dFinal 终止轮数 */
    quint64 sipHashImpl(const QByteArray& data, int cRound, int dFinal);

    quint64 m_k0;                   ///< 密钥低64位
    quint64 m_k1;                   ///< 密钥高64位
    QElapsedTimer m_timer;          ///< 计时器
    Stats m_stats;                  ///< 统计信息
    double m_timeSum;               ///< 累计处理时间
};

#endif // SIPHASH_H
