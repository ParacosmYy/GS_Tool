/**
 * @file TimSort.h
 * @brief TimSort排序 — 自适应归并排序与Galloping模式
 *
 * 功能: 实现 TimSort 排序算法，结合归并排序和插入排序的优点，
 *       利用数据的自然有序性(run)进行自适应排序。支持 Galloping
 *       模式加速两个有序序列的归并操作。
 *
 * 协作: IntroSort(内省排序) / MergeSorter(稳定归并)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <functional>

/**
 * @brief TimSort排序器
 */
class TimSort : public QObject {
    Q_OBJECT

public:
    /** @brief 统计信息 */
    struct Stats {
        quint64 totalSorted = 0;           ///< 累计排序次数
        quint64 totalElements = 0;         ///< 累计排序元素数
        double  avgProcessingTimeMs = 0.0;  ///< 平均处理时间(ms)
    };

    /** @brief 比较器类型 */
    using Comparator = std::function<bool(double, double)>;

    /**
     * @brief 构造函数
     * @param parent 父对象
     */
    explicit TimSort(QObject* parent = nullptr);

    /**
     * @brief 升序排序
     * @param data 待排序数组(原地排序)
     */
    void sort(QVector<double>& data);

    /**
     * @brief 自定义比较器排序
     * @param data 待排序数组
     * @param cmp 比较函数(cmp(a,b)=true 则a在前)
     */
    void sortWithComparator(QVector<double>& data,
                             std::function<bool(double, double)> cmp);

    /**
     * @brief 返回排序后的索引(不修改原数组)
     * @param data 输入数据
     * @return 排序后的索引数组
     *
     * 返回索引数组 indices[]，使得 data[indices[0]] <= data[indices[1]] <= ...
     */
    QVector<int> sortIndices(const QVector<double>& data);

    /**
     * @brief 检查数组是否已排序
     * @param data 输入数组
     * @return true 如果已升序排列
     */
    bool isSorted(const QVector<double>& data) const;

    /** @brief 获取统计 */
    const Stats& stats() const { return m_stats; }

    /** @brief 重置统计 */
    void resetStatistics();

signals:
    /** @brief 排序完成 @param count 排序元素数量 */
    void sortCompleted(int count);

private:
    /** @brief 最小run长度(32~64之间，取决于数组大小) */
    static const int kMinRun = 32;

    /**
     * @brief Run描述符 — 记录有序子序列的起始和长度
     */
    struct Run {
        int start;  ///< 起始索引
        int length; ///< 长度
    };

    /**
     * @brief 计算最小run长度
     * @param n 数组大小
     * @return 最小run长度(32~64)
     */
    static int calcMinRun(int n);

    /**
     * @brief 二分插入排序(对run内的小数组)
     * @param arr 数组 @param left 左边界 @param right 右边界 @param cmp 比较器
     */
    void binaryInsertionSort(QVector<double>& arr, int left, int right,
                               const Comparator& cmp);

    /**
     * @brief 向右扩展run的结尾
     * @param arr 数组 @param start 起始位置 @param end 数组末尾 @param cmp 比较器
     * @return run的结束位置(不含)
     */
    int extendRun(QVector<double>& arr, int start, int end,
                   const Comparator& cmp);

    /**
     * @brief 归并两个相邻run(带Galloping模式)
     * @param arr 数组 @param runA 第一个run @param runB 第二个run @param cmp 比较器
     */
    void mergeAt(QVector<double>& arr, const Run& runA, const Run& runB,
                  const Comparator& cmp);

    /**
     * @brief Galloping模式搜索 — 查找元素在有序范围中的插入位置
     * @param arr 数组 @param key 目标值 @param base 范围起始 @param len 范围长度
     * @param cmp 比较器 @param findMax true找上界，false找下界
     * @return 插入位置
     */
    int gallopSearch(const QVector<double>& arr, double key,
                      int base, int len, const Comparator& cmp,
                      bool findMax) const;

    double    m_timeSum;   ///< 处理时间累加器
    quint64   m_elemSum;   ///< 元素数量累加器
    Stats     m_stats;     ///< 统计信息
};
