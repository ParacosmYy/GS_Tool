#include "BinaryHeap11.h"
#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/**
 * @brief 构造函数
 * @param parent 父对象指针
 */
BinaryHeap11::BinaryHeap11(QObject* parent) : QObject(parent) {}

/**
 * @brief 重置所有统计信息
 */
void BinaryHeap11::resetStatistics() { m_stats = Stats(); m_timeSum = 0.0; }

/* 下沉操作 */
static void heapSiftDown11(QVector<double>& data, int i, int n, bool isMin)
{
    while (true) {
        int target = i;
        int left = 2 * i + 1;
        int right = 2 * i + 2;
        if (left < n) {
            if (isMin && data[left] < data[target]) target = left;
            if (!isMin && data[left] > data[target]) target = left;
        }
        if (right < n) {
            if (isMin && data[right] < data[target]) target = right;
            if (!isMin && data[right] > data[target]) target = right;
        }
        if (target == i) break;
        std::swap(data[i], data[target]);
        i = target;
    }
}

/* 上浮操作 */
static void heapSiftUp11(QVector<double>& data, int i, bool isMin)
{
    while (i > 0) {
        int parent = (i - 1) / 2;
        if (isMin && data[i] < data[parent]) { std::swap(data[i], data[parent]); i = parent; }
        else if (!isMin && data[i] > data[parent]) { std::swap(data[i], data[parent]); i = parent; }
        else break;
    }
}

/* 当前堆数据 */
static QVector<double> heapData11;
static bool heapIsMin11 = true;

/**
 * @brief 从数组构建堆（线性时间建堆）
 * @param data 输入数据
 * @param isMinHeap true为最小堆，false为最大堆
 */
void BinaryHeap11::buildHeap(const QVector<double>& data, bool isMinHeap)
{
    QElapsedTimer timer;
    timer.start();

    heapIsMin11 = isMinHeap;
    heapData11 = data;
    int n = heapData11.size();

    /* 从最后一个非叶节点开始下沉 */
    for (int i = n / 2 - 1; i >= 0; --i)
        heapSiftDown11(heapData11, i, n, heapIsMin11);

    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.totalHeapOps++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalHeapOps;
    emit heapOperationCompleted(heapData11.size());
}

/**
 * @brief 插入元素到堆中
 * @param value 待插入的值
 */
void BinaryHeap11::insert(double value)
{
    QElapsedTimer timer;
    timer.start();

    heapData11.append(value);
    heapSiftUp11(heapData11, heapData11.size() - 1, heapIsMin11);

    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.totalHeapOps++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalHeapOps;
    emit heapOperationCompleted(heapData11.size());
}

/**
 * @brief 弹出堆顶元素
 * @return 堆顶元素值
 */
double BinaryHeap11::extractTop()
{
    QElapsedTimer timer;
    timer.start();

    if (heapData11.isEmpty()) {
        qint64 elapsed = timer.elapsed();
        m_timeSum += elapsed; m_stats.totalHeapOps++;
        m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalHeapOps;
        emit heapOperationCompleted(0);
        return 0.0;
    }

    double top = heapData11[0];
    heapData11[0] = heapData11.last();
    heapData11.removeLast();
    if (!heapData11.isEmpty())
        heapSiftDown11(heapData11, 0, heapData11.size(), heapIsMin11);

    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.totalHeapOps++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalHeapOps;
    emit heapOperationCompleted(heapData11.size());
    return top;
}

/**
 * @brief 堆排序，返回排序后的数组
 * @param data 待排序数据
 * @param ascending 是否升序排列
 * @return 排序后的数组
 */
QVector<double> BinaryHeap11::heapSort(const QVector<double>& data, bool ascending)
{
    QElapsedTimer timer;
    timer.start();

    QVector<double> result = data;
    int n = result.size();
    /* 升序用最大堆，降序用最小堆 */
    bool useMax = ascending;

    /* 建堆 */
    for (int i = n / 2 - 1; i >= 0; --i)
        heapSiftDown11(result, i, n, !useMax);

    /* 逐个取出 */
    for (int i = n - 1; i > 0; --i) {
        std::swap(result[0], result[i]);
        heapSiftDown11(result, 0, i, !useMax);
    }

    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.totalHeapOps++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalHeapOps;
    emit heapOperationCompleted(data.size());
    return result;
}

/**
 * @brief 获取当前堆大小
 * @return 堆中元素数量
 */
int BinaryHeap11::size() const
{
    return heapData11.size();
}
