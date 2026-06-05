/**
 * @file BinaryHeap2.h
 * @brief 增强二叉堆 — 支持decrease-key与merge操作
 *
 * 在标准二叉最小堆基础上增加: decrease-key(降低键值)、
 * increase-key(升高键值)、merge(合并两个堆)、批量建堆。
 * 适用于Dijkstra、Huffman、事件调度等场景。
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QHash>
#include <QVariant>

/**
 * @class BinaryHeap2
 * @brief 增强二叉最小堆 — decrease-key + merge
 *
 * 每个元素通过Handle标识, 可通过Handle高效修改键值。
 * merge操作将另一个堆的所有元素合并到当前堆。
 */
class BinaryHeap2 : public QObject
{
    Q_OBJECT

public:
    /** @brief 元素句柄 */
    using Handle = int;

    /** @brief 统计信息 */
    struct Stats {
        quint64 totalInserts = 0;       ///< 总插入次数
        quint64 totalExtracts = 0;      ///< 总提取次数
        quint64 totalMerges = 0;        ///< 总合并次数
        double  avgProcessingTimeMs = 0.0; ///< 平均操作耗时(ms)
    };

    /** @brief 构造函数 @param parent 父对象 */
    explicit BinaryHeap2(QObject* parent = nullptr);

    /**
     * @brief 插入元素
     * @param key 优先级键(越小越优先)
     * @param value 关联值
     * @return 元素句柄
     */
    Handle insert(double key, const QVariant& value);

    /**
     * @brief 批量建堆
     * @param items (key, value) 列表
     * @return 所有元素句柄列表
     */
    QVector<Handle> buildHeap(const QVector<QPair<double, QVariant>>& items);

    /**
     * @brief 提取最小元素
     * @return [key, value] 对; 堆空返回无效QVariant
     */
    QPair<double, QVariant> extractMin();

    /**
     * @brief 降低键值(新键必须小于当前键)
     * @param handle 元素句柄
     * @param newKey 新键值
     */
    void decreaseKey(Handle handle, double newKey);

    /**
     * @brief 升高键值(新键必须大于当前键)
     * @param handle 元素句柄
     * @param newKey 新键值
     */
    void increaseKey(Handle handle, double newKey);

    /**
     * @brief 合并另一个堆到当前堆
     * @param other 被合并的堆(合并后other变空)
     */
    void merge(BinaryHeap2& other);

    /** @brief 查看堆顶 @return [key, value] */
    QPair<double, QVariant> peek() const;

    /** @brief 堆是否为空 */
    bool isEmpty() const;

    /** @brief 堆大小 */
    int size() const;

    /** @brief 获取统计 */
    Stats stats() const;

    /** @brief 重置统计 */
    void resetStatistics();

signals:
    /** @brief 元素插入 @param handle 句柄 */
    void elementInserted(Handle handle);
    /** @brief 元素提取 @param key 键值 */
    void elementExtracted(double key);
    /** @brief 合并完成 @param count 合并元素数 */
    void mergeCompleted(int count);

private:
    /** @brief 堆节点 */
    struct Node {
        double key;         ///< 优先级键
        QVariant value;     ///< 关联值
        int heapIndex = -1; ///< 堆数组位置
    };

    void siftUp(int index);  ///< 上浮
    void siftDown(int index); ///< 下沉
    void swapNodes(int i, int j); ///< 交换

    QVector<Node> m_nodes;    ///< 所有节点
    QVector<int> m_heap;      ///< 堆数组(存节点索引)
    int m_nextHandle = 0;     ///< 下一个句柄
    Stats m_stats;            ///< 统计
    double m_timeSum = 0.0;   ///< 累计耗时
};
