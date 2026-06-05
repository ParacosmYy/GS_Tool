/**
 * @file BitAllocator.h
 * @brief 位级内存分配器 — 位范围分配与碎片统计
 *
 * 功能: 从位池中分配/释放位范围，支持首次适配/最佳适配策略，
 *       提供碎片率统计，用于寄存器位域管理和协议字段分配。
 *
 * 协作: DataMaskEditor(位掩码) / PacketBuilder(协议字段)
 */
#ifndef BITALLOCATOR_H
#define BITALLOCATOR_H

#include <QObject>
#include <QVector>
#include <QMap>

/**
 * @class BitAllocator
 * @brief 位级内存池分配器
 */
class BitAllocator : public QObject {
    Q_OBJECT

public:
    /** 分配策略 */
    enum class Strategy {
        FirstFit,      ///< 首次适配 — 找到第一个足够大的空闲块
        BestFit        ///< 最佳适配 — 找到最小的足够大空闲块
    };

    /** 分配统计 */
    struct Stats {
        quint64 totalAllocs = 0;              ///< 总分配次数
        quint64 totalFrees = 0;               ///< 总释放次数
        double  fragmentationRatio = 0.0;     ///< 碎片率(0~1)
        quint64 peakUsage = 0;                ///< 峰值使用位数
    };

    /** 分配句柄 */
    struct AllocHandle {
        int startBit = -1;                    ///< 起始位索引
        int bitCount = 0;                     ///< 分配的位数
        bool valid = false;                   ///< 是否有效
    };

    /** 空闲块描述 */
    struct FreeBlock {
        int startBit;                         ///< 起始位
        int bitCount;                         ///< 空闲位数
    };

    /**
     * @brief 构造函数
     * @param totalBits 位池总位数
     * @param parent 父对象
     */
    explicit BitAllocator(int totalBits = 256, QObject* parent = nullptr);

    /** @brief 设置分配策略 @param strategy 策略类型 */
    void setStrategy(Strategy strategy);

    /** @brief 分配指定数量的位 @param bitCount 位数 @return 分配句柄 */
    AllocHandle allocate(int bitCount);

    /** @brief 释放分配 @param handle 分配句柄 */
    void deallocate(const AllocHandle& handle);

    /** @brief 获取所有空闲块 @return 空闲块列表 */
    QVector<FreeBlock> freeBlocks() const;

    /** @brief 获取所有已分配块 @return 已分配块列表 */
    QVector<AllocHandle> allocatedBlocks() const;

    /** @brief 获取已使用位数 @return 使用位数 */
    int usedBits() const;

    /** @brief 获取空闲位数 @return 空闲位数 */
    int freeBits() const;

    /** @brief 获取总位数 @return 总位数 */
    int totalBits() const;

    /** @brief 重置分配器(释放所有) */
    void reset();

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 分配完成 @param handle 分配句柄 */
    void allocated(const AllocHandle& handle);

    /** @brief 释放完成 @param startBit 起始位 @param bitCount 位数 */
    void deallocated(int startBit, int bitCount);

    /** @brief 碎片率告警 @param ratio 碎片率 */
    void fragmentationWarning(double ratio);

private:
    void recalculateFragmentation();    ///< 重算碎片率

    int m_totalBits;                       ///< 总位数
    Strategy m_strategy;                   ///< 分配策略
    QVector<bool> m_pool;                  ///< 位池(true=已分配)
    QMap<int, int> m_allocations;          ///< startBit → bitCount

    Stats m_stats;
    int m_usedBits;                        ///< 已使用位数
};

#endif // BITALLOCATOR_H
