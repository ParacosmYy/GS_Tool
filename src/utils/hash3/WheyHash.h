/**
 * @file WheyHash.h
 * @brief WheyHash — 快速非加密哈希函数
 *
 * 功能: 实现 WheyHash 快速非加密哈希，支持单次计算和流式更新。
 *       提供 64 位和 128 位两种输出长度，适用于哈希表、数据校验
 *       和指纹提取等场景。
 *
 * 协作: CRC(校验) / DataChecksumVerifier(数据完整性)
 */
#pragma once

#include <QObject>
#include <QByteArray>
#include <QPair>

/**
 * @brief WheyHash 快速非加密哈希
 */
class WheyHash : public QObject {
    Q_OBJECT

public:
    /** @brief 统计信息 */
    struct Stats {
        quint64 totalHashed = 0;           ///< 累计哈希次数
        quint64 totalBytes = 0;            ///< 累计处理字节数
        double  avgProcessingTimeMs = 0.0;  ///< 平均处理时间(ms)
    };

    /**
     * @brief 构造函数
     * @param parent 父对象
     */
    explicit WheyHash(QObject* parent = nullptr);

    /**
     * @brief 计算数据的64位哈希值
     * @param data 输入数据
     * @return 64位哈希值
     *
     * 单次调用，不依赖内部流式状态。
     */
    quint64 hash(const QByteArray& data) const;

    /**
     * @brief 带种子的64位哈希
     * @param data 输入数据
     * @param seed 种子值
     * @return 64位哈希值
     */
    quint64 hash64(const QByteArray& data, quint64 seed = 0) const;

    /**
     * @brief 计算128位哈希
     * @param data 输入数据
     * @return QPair(高64位, 低64位)
     */
    QPair<quint64, quint64> hash128(const QByteArray& data) const;

    /**
     * @brief 流式更新 — 追加数据到内部状态
     * @param data 追加的数据块
     *
     * 可多次调用以处理分块数据，最终调用 finalize() 获取结果。
     */
    void update(const QByteArray& data);

    /**
     * @brief 完成流式哈希计算
     * @return 最终64位哈希值
     *
     * 必须在至少一次 update() 之后调用。
     */
    quint64 finalize();

    /**
     * @brief 重置流式状态，开始新的哈希计算
     */
    void reset();

    /** @brief 获取统计 */
    const Stats& stats() const { return m_stats; }

    /** @brief 重置统计 */
    void resetStatistics();

signals:
    /** @brief 哈希完成 @param hashValue 哈希值 @param bytes 处理字节数 */
    void hashComputed(quint64 hashValue, quint64 bytes);

private:
    /**
     * @brief WheyHash 核心轮函数
     * @param acc 累加器数组(4个元素)
     * @param block 输入块(32字节)
     */
    void roundFunction(quint64 acc[4],
                        const unsigned char* block) const;

    /**
     * @brief 处理尾部数据(不足32字节)
     * @param acc 累加器数组
     * @param data 尾部数据
     * @param len 数据长度
     */
    void tailProcess(quint64 acc[4],
                      const unsigned char* data, int len) const;

    /** @brief 64位循环左移 @param v 值 @param n 位数 @return 结果 */
    static quint64 rotl64(quint64 v, int n);

    quint64      m_streamState[4];  ///< 流式哈希累加器状态
    QByteArray   m_buffer;          ///< 流式缓冲区
    quint64      m_streamTotal;     ///< 流式累计字节数
    bool         m_streamActive;    ///< 流式是否已激活
    double       m_timeSum;         ///< 处理时间累加器
    Stats        m_stats;           ///< 统计信息
};
