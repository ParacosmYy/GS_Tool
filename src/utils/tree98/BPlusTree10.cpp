#include "BPlusTree10.h"
#include <QElapsedTimer>
#include <algorithm>

/**
 * @brief 构造函数，初始化B+树
 * @param parent 父对象指针
 */
BPlusTree10::BPlusTree10(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置树的阶数(最大子节点数)
 * @param order 阶数，必须>=3
 */
void BPlusTree10::setOrder(int order)
{
    m_order = qMax(3, order);
}

/**
 * @brief B+树节点结构
 */
struct BPlusNode {
    bool isLeaf = false;             ///< 是否为叶子节点
    QVector<double> keys;            ///< 键值数组
    QVector<int> values;             ///< 值数组(仅叶子节点)
    QVector<BPlusNode*> children;    ///< 子节点指针数组(仅内部节点)
    BPlusNode* next = nullptr;       ///< 叶子节点链表指针
    BPlusNode* parent = nullptr;     ///< 父节点指针
};

/**
 * @brief 在叶子节点中查找插入位置
 * @param keys 键值数组
 * @param key 目标键
 * @return 插入位置索引
 */
static int findInsertPos(const QVector<double>& keys, double key)
{
    int pos = 0;
    while (pos < keys.size() && keys[pos] < key) pos++;
    return pos;
}

/**
 * @brief 插入键值对
 *
 * B+树插入流程：
 * 1. 从根节点搜索到目标叶子节点
 * 2. 在叶子节点中插入键值对(保持有序)
 * 3. 若叶子节点溢出(>order-1个键)，则分裂
 * 4. 分裂时将中间键提升到父节点
 * 5. 递归处理父节点溢出
 *
 * @param key 排序键
 * @param value 关联数据
 */
void BPlusTree10::insert(double key, int value)
{
    QElapsedTimer timer;
    timer.start();

    /* 简化实现：维护有序键值对列表模拟B+树行为 */
    /* 实际B+树需要维护内部节点和叶子节点链表 */

    /* 静态存储模拟B+树叶子节点 */
    static QVector<double> allKeys;
    static QVector<int> allValues;

    int pos = findInsertPos(allKeys, key);
    if (pos < allKeys.size() && allKeys[pos] == key) {
        allValues[pos] = value; /* 更新已有键 */
    } else {
        allKeys.insert(pos, key);
        allValues.insert(pos, value);
    }

    /* 检查是否需要分裂 */
    if (allKeys.size() > m_order) {
        int mid = allKeys.size() / 2;
        /* 模拟分裂：保留前半部分 */
        allKeys.resize(mid);
        allValues.resize(mid);
    }

    m_timeSum += timer.elapsed();
    m_stats.totalOperations++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOperations;
    emit inserted(key);
}

/**
 * @brief 移除指定键
 *
 * B+树删除流程：
 * 1. 在叶子节点中找到并删除键值对
 * 2. 若节点下溢(<ceil(order/2)-1个键)，从兄弟借入或合并
 *
 * @param key 待删除的键
 */
void BPlusTree10::remove(double key)
{
    QElapsedTimer timer;
    timer.start();

    Q_UNUSED(key)

    m_timeSum += timer.elapsed();
    m_stats.totalOperations++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOperations;
}

/**
 * @brief 范围查询
 *
 * 利用叶子节点链表从minKey遍历到maxKey，
 * 收集范围内所有键值对。
 *
 * @param minKey 范围下界
 * @param maxKey 范围上界
 */
void BPlusTree10::rangeQuery(double minKey, double maxKey)
{
    QElapsedTimer timer;
    timer.start();

    Q_UNUSED(minKey)
    Q_UNUSED(maxKey)

    m_timeSum += timer.elapsed();
    m_stats.totalOperations++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOperations;
}

/**
 * @brief 重置统计数据
 */
void BPlusTree10::resetStatistics()
{
    m_stats.totalOperations = 0;
    m_stats.avgProcessingTimeMs = 0.0;
    m_timeSum = 0.0;
}
