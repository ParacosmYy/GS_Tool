/**
 * @file TimSort.h
 * @brief TimSort混合稳定排序 — 归并+插入混合排序算法
 *
 * 提供TimSort混合稳定排序实现, 结合归并排序和插入排序,
 * 对部分有序数据具有优异性能, 适用于嵌入式调试场景中的
 * 数据排序、排名计算和统计分位数求解。
 */
#ifndef TIM_SORT_H
#define TIM_SORT_H

#include <QObject>
#include <QVector>
#include <functional>

/**
 * @class TimSort
 * @brief TimSort混合稳定排序器
 *
 * 典型用法:
 * @code
 *   TimSort sorter;
 *   QVector<double> data = {3.0, 1.0, 4.0, 1.5};
 *   sorter.sort(data);
 * @endcode
 */
class TimSort : public QObject {
    Q_OBJECT

public:
    /** @brief 操作统计结构 */
    struct Stats {
        quint64 totalSorts = 0;            ///< 排序操作总次数
        quint64 totalComparisons = 0;      ///< 比较操作总次数
        double  avgProcessingTimeMs = 0.0; ///< 平均处理耗时(ms)
    };

    /** @brief 构造函数 @param parent 父对象 */
    explicit TimSort(QObject* parent = nullptr);

    /** @brief 析构函数 */
    ~TimSort() override;

    // ── 排序 ──

    /**
     * @brief 对double数组进行TimSort排序(升序)
     * @param data 待排序数组(原地排序)
     */
    void sort(QVector<double>& data);

    /**
     * @brief 对任意类型数组进行TimSort排序(带自定义比较器)
     * @tparam T 元素类型
     * @param data 待排序数组(原地排序)
     * @param comp 比较器, 返回true表示a<b
     */
    template<typename T>
    void sortWithComparator(QVector<T>& data,
                            const std::function<bool(const T&, const T&)>& comp);

    // ── 统计 ──

    /** @brief 获取当前统计数据快照 */
    Stats stats() const;

    /** @brief 重置所有统计计数器归零 */
    void resetStatistics();

signals:
    /** @brief 排序完成信号 @param count 排序元素数 */
    void sortCompleted(int count);

private:
    /**
     * @brief 获取或计算最小run长度
     * @param n 剩余元素数
     * @return 最小run长度
     */
    int minRunLength(int n) const;

    /**
     * @brief 对指定范围执行插入排序
     * @param data 数据数组
     * @param left 左边界(含)
     * @param right 右边界(含)
     */
    void insertionSort(QVector<double>& data, int left, int right);

    /**
     * @brief 对指定范围执行插入排序(模板版本)
     */
    template<typename T>
    void insertionSortImpl(QVector<T>& data, int left, int right,
                           const std::function<bool(const T&, const T&)>& comp);

    /**
     * @brief 合并相邻的两个有序run
     * @param data 数据数组
     * @param l 第一个run起始
     * @param m 第一个run结束
     * @param r 第二个run结束
     */
    void merge(QVector<double>& data, int l, int m, int r);

    /**
     * @brief 合并相邻的两个有序run(模板版本)
     */
    template<typename T>
    void mergeImpl(QVector<T>& data, int l, int m, int r,
                   const std::function<bool(const T&, const T&)>& comp);

    /**
     * @brief 更新平均处理时间
     * @param elapsedMs 本次耗时(ms)
     */
    void updateAvgTime(double elapsedMs) const;

    /** @brief 最小run长度常数 */
    static constexpr int MIN_MERGE = 32;

    /** @brief 操作统计(mutable支持const方法更新) */
    mutable Stats m_stats;
};

// ── 模板方法实现 ──
#include "TimSort.ipp"

#endif // TIM_SORT_H
