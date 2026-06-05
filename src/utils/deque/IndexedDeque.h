/**
 * @file IndexedDeque.h
 * @brief 索引双端队列 — 基于环形缓冲区的O(1)双端操作
 *
 * 功能: O(1)前插/后插/前删/后删，O(1)索引访问，
 *       自动扩容环形缓冲区，支持迭代器遍历，
 *       统计操作次数/容量/耗时。
 */
#ifndef INDEXEDDEQUE_H
#define INDEXEDDEQUE_H

#include <QObject>
#include <QVector>

/**
 * @brief 索引双端队列 — 环形缓冲区实现
 */
class IndexedDeque : public QObject {
    Q_OBJECT

public:
    /** @brief 统计 */
    struct Stats {
        quint64 totalPushFront = 0;     ///< 累计前插次数
        quint64 totalPushBack = 0;      ///< 累计后插次数
        quint64 totalPopFront = 0;      ///< 累计前删次数
        quint64 totalPopBack = 0;       ///< 累计后删次数
        int     currentSize = 0;        ///< 当前元素数
        int     capacity = 0;           ///< 当前缓冲区容量
        double  avgProcessingTimeMs = 0.0; ///< 平均处理时间
    };

    explicit IndexedDeque(int initialCapacity = 64, QObject* parent = nullptr);

    /** @brief 前端插入 @param value 元素值 */
    void pushFront(double value);

    /** @brief 后端插入 @param value 元素值 */
    void pushBack(double value);

    /** @brief 前端弹出 @return 弹出的元素值 */
    double popFront();

    /** @brief 后端弹出 @return 弹出的元素值 */
    double popBack();

    /** @brief 索引访问 @param index 索引 @return 元素值 */
    double at(int index) const;

    /** @brief 首元素 @return 前端元素 */
    double front() const;

    /** @brief 末元素 @return 后端元素 */
    double back() const;

    /** @brief 当前元素数 @return 大小 */
    int size() const;

    /** @brief 是否为空 @return 空判断 */
    bool isEmpty() const;

    /** @brief 清空队列 */
    void clear();

    /** @brief 转为QVector @return 元素向量(从前到后) */
    QVector<double> toVector() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 元素入队 @param front 是否前端 @param value 元素值 */
    void elementPushed(bool front, double value);
    /** @brief 元素出队 @param front 是否前端 @param value 元素值 */
    void elementPopped(bool front, double value);

private:
    /** @brief 扩容(2倍) */
    void resizeBuffer(int newCapacity);
    /** @brief 物理索引转逻辑索引 */
    int logicalIndex(int physical) const;

    QVector<double> m_buffer;       ///< 环形缓冲区
    int m_head;                     ///< 队首物理索引
    int m_tail;                     ///< 队尾物理索引(下一个空位)
    int m_count;                    ///< 当前元素数
    Stats m_stats;
    double m_timeSum;               ///< 处理时间累加器
};

#endif // INDEXEDDEQUE_H
