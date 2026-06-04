/**
 * @file DataHistoryBuffer.h
 * @brief 数据历史环形缓冲区 -- 时间/大小双策略保留的数据存储
 *
 * 泛型环形缓冲区模板，为数据历史特性提供底层存储支撑。
 * 支持容量+年龄双策略数据保留，O(1)入队出队，二分查找时间范围查询，
 * 批量入队、压缩、最近N条查询、CSV格式导出，序列号+时间戳双索引。
 * 模板参数T需支持默认构造和拷贝赋值。非线程安全，外部需自行加锁。
 */
#ifndef DATAHISTORYBUFFER_H
#define DATAHISTORYBUFFER_H

#include <QVector>
#include <QString>
#include <QFile>
#include <QTextStream>
#include <QDateTime>
#include <functional>

/**
 * @class DataHistoryBuffer
 * @brief 数据历史环形缓冲区模板
 *
 * 预分配QVector底层存储，read/write双索引实现O(1)入队出队。
 * 序列号单调递增，时间范围查询使用二分查找保证O(logN)效率。
 */
template<typename T>
class DataHistoryBuffer : public QObject {
public:
    /** @brief 单条数据条目，携带数据、时间戳和序列号 */
    struct Entry {
        T data;                  ///< 用户数据
        qint64 timestampMs;      ///< 入队时的毫秒级时间戳(epoch)
        qint64 sequenceNumber;   ///< 全局单调递增序列号
    };

    /** @brief 数据保留策略配置 */
    struct RetentionPolicy {
        int maxEntries = 10000;    ///< 最大条目数，超过时按策略淘汰
        qint64 maxAgeMs = 3600000; ///< 最大存活时间(毫秒)，0表示不限
        bool dropOldest = true;    ///< true=淘汰最旧，false=拒绝新数据
    };

    /** @brief 运行时统计信息 */
    struct Stats {
        quint64 totalPushed = 0;       ///< 累计入队条目数
        quint64 totalPopped = 0;       ///< 累计出队条目数(手动pop)
        quint64 totalDropped = 0;      ///< 累计因策略淘汰的条目数
        quint64 totalQueries = 0;      ///< 累计查询调用次数
        quint64 totalExports = 0;      ///< 累计导出调用次数
        quint64 totalCompressions = 0; ///< 累计压缩调用次数
        quint64 bytesStored = 0;       ///< 当前存储估算字节数(条目数*sizeof(Entry))
        int peakSize = 0;              ///< 历史峰值条目数
        int currentSize = 0;           ///< 当前有效条目数
        double fillRatio = 0.0;        ///< 当前填充率(0.0~1.0)
    };

    /** @brief 构造函数 @param capacity 预分配容量 @param parent QObject父对象 */
    explicit DataHistoryBuffer(int capacity = 10000, QObject* parent = nullptr);
    ~DataHistoryBuffer() override = default;

    DataHistoryBuffer(const DataHistoryBuffer&) = delete;
    DataHistoryBuffer& operator=(const DataHistoryBuffer&) = delete;
    DataHistoryBuffer(DataHistoryBuffer&&) = default;
    DataHistoryBuffer& operator=(DataHistoryBuffer&&) = default;

    // ── 数据写入 ──
    /** @brief 写入单条数据 @param item 用户数据 */
    void push(const T& item);
    /** @brief 批量写入数据 @param items 数据列表 */
    void pushBatch(const QVector<T>& items);

    // ── 数据读取 ──
    /** @brief 弹出最旧条目。空缓冲区返回Entry{sequenceNumber=-1} */
    Entry popOldest();
    /** @brief 获取最新条目(不移除)。空缓冲区返回Entry{sequenceNumber=-1} */
    Entry newest() const;
    /** @brief 获取最旧条目(不移除)。空缓冲区返回Entry{sequenceNumber=-1} */
    Entry oldest() const;

    // ── 范围查询 ──
    /** @brief 按序列号范围查询[fromSeq,toSeq]，返回升序Entry列表 */
    QVector<Entry> range(qint64 fromSeq, qint64 toSeq) const;
    /** @brief 按时间范围查询[fromMs,toMs]，内部二分查找定位，返回升序Entry列表 */
    QVector<Entry> timeRange(qint64 fromMs, qint64 toMs) const;
    /** @brief 获取最近N条数据(按时间升序)，count超过当前大小时返回全部 */
    QVector<Entry> lastN(int count) const;

    // ── 策略与状态 ──
    /** @brief 设置数据保留策略 @param policy 新的保留策略 */
    void setRetentionPolicy(const RetentionPolicy& policy);
    /** @brief 获取当前保留策略的只读引用 */
    const RetentionPolicy& retentionPolicy() const;
    /** @brief 当前有效条目数 */
    int size() const;
    /** @brief 预分配容量 */
    int capacity() const;
    /** @brief 缓冲区是否为空 */
    bool isEmpty() const;
    /** @brief 清空所有数据并重置索引(序列号不重置) */
    void clear();

    // ── 数据处理 ──
    /** @brief 压缩数据：仅保留每keepEveryN条中的第1条 */
    void compress(int keepEveryN);
    /** @brief 导出数据到CSV文件 @return 成功导出条目数，-1=文件打开失败 */
    qint64 exportToCsv(const QString& filePath,
                       std::function<QString(const T&)> formatter);

    // ── 统计 ──
    /** @brief 获取运行时统计信息的只读引用 */
    const Stats& stats() const;
    /** @brief 重置所有统计计数器(不影响数据和索引) */
    void resetStatistics();

signals:
    /** @brief 缓冲区溢出信号 @param dropped 淘汰的条目数量 */
    void bufferOverflow(int dropped);
    /** @brief 新条目入队信号 @param seq 新条目的序列号 */
    void entryPushed(qint64 seq);
    /** @brief 缓冲区已清空信号 */
    void bufferCleared();

private:
    void applyRetentionPolicy();   ///< 应用保留策略，淘汰过期/溢出数据
    void updateStatsDerived();     ///< 更新统计信息中的派生字段
    /** @brief 二分查找>=targetMs的最左逻辑索引，未找到返回m_count */
    int binarySearchByTime(qint64 targetMs) const;

    QVector<Entry> m_buffer;       ///< 底层预分配存储
    int m_capacity;                ///< 预分配容量
    int m_readIdx;                 ///< 读索引(最旧元素位置)
    int m_writeIdx;                ///< 写索引(下一个写入位置)
    int m_count;                   ///< 当前有效条目数
    qint64 m_nextSeq;              ///< 下一个分配的序列号
    RetentionPolicy m_policy;      ///< 数据保留策略
    mutable Stats m_stats;         ///< 运行时统计(mutable支持const方法内递增查询计数)
};

// 包含内联实现
#include "DataHistoryBuffer.ipp"

#endif // DATAHISTORYBUFFER_H
