/**
 * @file IndexedDeque.h
 * @brief 索引双端队列(Indexed Deque)
 */

#pragma once

#include <QObject>
#include <QVector>
#include <QVariant>

/**
 * @class IndexedDeque
 * @brief 支持O(1)索引访问的双端队列
 *
 * 基于环形缓冲区实现，支持O(1)首尾插入/删除和O(1)索引访问。
 * 适用于滑动窗口、流水线缓冲等场景。
 */
class IndexedDeque : public QObject
{
    Q_OBJECT

public:
    /** @brief 统计信息 */
    struct Stats {
        int totalPushFront = 0;     /**< 前端推入次数 */
        int totalPushBack = 0;      /**< 后端推入次数 */
        int totalPopFront = 0;      /**< 前端弹出次数 */
        int totalPopBack = 0;       /**< 后端弹出次数 */
        int totalAccess = 0;        /**< 索引访问次数 */
        double avgProcessingTimeMs = 0.0; /**< 平均处理时间(ms) */
    };

    /**
     * @brief 构造函数
     * @param capacity 初始容量(默认1024)
     * @param parent 父对象
     */
    explicit IndexedDeque(int capacity = 1024, QObject* parent = nullptr);

    /** @brief 前端推入 */
    void pushFront(const QVariant& value);

    /** @brief 后端推入 */
    void pushBack(const QVariant& value);

    /** @brief 前端弹出 */
    QVariant popFront();

    /** @brief 后端弹出 */
    QVariant popBack();

    /** @brief 索引访问(0=首元素) */
    QVariant at(int index) const;

    /** @brief 索引设置 */
    void setAt(int index, const QVariant& value);

    /** @brief 获取前端元素(不弹出) */
    QVariant front() const;

    /** @brief 获取后端元素(不弹出) */
    QVariant back() const;

    /** @brief 元素数量 */
    int size() const;

    /** @brief 是否为空 */
    bool isEmpty() const;

    /** @brief 清空 */
    void clear();

    /** @brief 转为QVector */
    QVector<QVariant> toVector() const;

    /** @brief 获取统计 */
    Stats stats() const;

    /** @brief 重置统计 */
    void resetStatistics();

signals:
    /** @brief 元素推入信号 */
    void elementAdded(int newSize);

private:
    void ensureCapacity();
    int physicalIndex(int logicalIndex) const;

    QVector<QVariant> m_buffer;  /**< 环形缓冲区 */
    int m_front;                  /**< 前端索引 */
    int m_count;                  /**< 元素数量 */
    int m_capacity;               /**< 容量 */
    Stats m_stats;                /**< 统计 */
    double m_timeSum;             /**< 累计时间 */
};
