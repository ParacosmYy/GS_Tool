/**
 * @file RabinFingerprint.h
 * @brief Rabin指纹(Rabin Fingerprint)
 */

#pragma once

#include <QObject>
#include <QByteArray>
#include <QVector>

/**
 * @class RabinFingerprint
 * @brief Rabin指纹 — 基于多项式哈希的滑动窗口指纹
 *
 * 支持可变窗口大小、多项式模运算、内容定义分块(CDC)。
 * 适用于数据去重、增量同步、内容分块等场景。
 */
class RabinFingerprint : public QObject
{
    Q_OBJECT

public:
    /** @brief 统计信息 */
    struct Stats {
        int totalUpdates = 0;     /**< 总更新次数 */
        int totalChunks = 0;      /**< 总分块数 */
        long long totalBytes = 0; /**< 总字节数 */
        double avgProcessingTimeMs = 0.0; /**< 平均处理时间(ms) */
    };

    /** @brief 构造函数 */
    explicit RabinFingerprint(int windowSize = 48, quint64 polynomial = 0x3DA3358B4DC173LL,
                                QObject* parent = nullptr);

    /**
     * @brief 滑动更新: 添加一个字节
     * @param byte 新字节
     * @return 当前指纹值
     */
    quint64 slide(quint8 byte);

    /**
     * @brief 计算整个数据的指纹
     * @param data 输入数据
     * @return 指纹值
     */
    quint64 fingerprint(const QByteArray& data);

    /**
     * @brief 内容定义分块(CDC)
     * @param data 输入数据
     * @param minChunk 最小块大小
     * @param maxChunk 最大块大小
     * @param mask 分块掩码
     * @return 分块边界位置列表
     */
    QVector<int> chunk(const QByteArray& data,
                        int minChunk = 1024, int maxChunk = 8192,
                        quint64 mask = 0x1FFF);

    /**
     * @brief 重置滑动窗口
     */
    void reset();

    /** @brief 获取当前指纹 */
    quint64 currentFingerprint() const;

    /** @brief 获取统计 */
    Stats stats() const;

    /** @brief 重置统计 */
    void resetStatistics();

signals:
    /** @brief 分块完成信号 */
    void chunkingCompleted(int chunkCount);

private:
    void buildTable();

    int m_windowSize;
    quint64 m_polynomial;
    quint64 m_fingerprint;
    QByteArray m_window;
    int m_windowPos;
    QVector<quint64> m_table;
    QVector<quint64> m_outTable;

    Stats m_stats;
    double m_timeSum;
};
