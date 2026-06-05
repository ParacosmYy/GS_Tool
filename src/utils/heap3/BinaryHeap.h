/**
 * @file BinaryHeap.h
 * @brief 通用二叉最小堆 — 支持decrease-key操作的优先队列
 *
 * 提供泛型二叉最小堆实现, 支持插入、提取最小值、decrease-key和查看堆顶,
 * 适用于Dijkstra最短路径、Huffman编码、事件调度等需要优先队列的场景。
 */
#ifndef BINARYHEAP_H
#define BINARYHEAP_H

#include <QObject>
#include <QVector>
#include <QHash>
#include <utility>

/**
 * @class BinaryHeap
 * @brief 通用二叉最小堆 — 泛型key-value优先队列
 *
 * 典型用法:
 * @code
 *   BinaryHeap heap;
 *   int h = heap.insert(3.0, "item1");
 *   auto top = heap.peek();
 *   heap.decreaseKey(h, 1.0);
 *   auto min = heap.extractMin();
 * @endcode
 */
class BinaryHeap : public QObject {
    Q_OBJECT

public:
    /** @brief 堆元素句柄(用于decrease-key) */
    using Handle = int;

    /** @brief 操作统计结构 */
    struct Stats {
        quint64 totalInserts = 0;       ///< 总插入次数
        quint64 totalExtracts = 0;      ///< 总提取次数
        double  avgProcessingTimeMs = 0.0; ///< 平均操作耗时(ms)
    };

    /** @brief 构造函数 @param parent 父对象 */
    explicit BinaryHeap(QObject* parent = nullptr);

    /** @brief 析构函数 */
    ~BinaryHeap() override;

    // ── 核心接口 ──

    /**
     * @brief 插入元素
     * @param key 优先级键(越小越优先)
     * @param value 关联值
     * @return 元素句柄(用于后续decreaseKey)
     */
    Handle insert(double key, const QVariant& value);

    /**
     * @brief 提取最小元素
     * @return [key, value] 对; 堆空时返回无效QVariant
     */
    std::pair<double, QVariant> extractMin();

    /**
     * @brief 降低指定元素的键值
     * @param handle 元素句柄(由insert返回)
     * @param newKey 新键值(必须小于当前键)
     */
    void decreaseKey(Handle handle, double newKey);

    /**
     * @brief 查看堆顶元素(不提取)
     * @return [key, value] 对; 堆空时返回无效QVariant
     */
    std::pair<double, QVariant> peek() const;

    /** @brief 堆是否为空 */
    bool isEmpty() const;

    /** @brief 堆中元素数量 */
    int size() const;

    // ── 统计 ──

    /** @brief 获取当前统计数据快照 */
    Stats stats() const;

    /** @brief 重置所有统计计数器归零 */
    void resetStatistics();

signals:
    /** @brief 元素插入信号 @param handle 元素句柄 */
    void elementInserted(Handle handle);
    /** @brief 元素提取信号 @param key 被提取元素的键 */
    void elementExtracted(double key);

private:
    /** @brief 堆节点 */
    struct Node {
        double key;       ///< 优先级键
        QVariant value;   ///< 关联值
        int heapIndex;    ///< 在堆数组中的位置
    };

    /** @brief 上浮操作 */
    void siftUp(int index);

    /** @brief 下沉操作 */
    void siftDown(int index);

    /** @brief 交换堆中两个位置 */
    void swapNodes(int i, int j);

    /** @brief 存储所有节点的列表 */
    QVector<Node> m_nodes;

    /** @brief 堆索引: heapPosition → 节点ID */
    QVector<int> m_heap;

    /** @brief 下一个可用句柄 */
    int m_nextHandle = 0;

    /** @brief 操作统计 */
    Stats m_stats;
};

#endif // BINARYHEAP_H
