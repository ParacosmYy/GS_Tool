/**
 * @file MergeSorter.h
 * @brief 归并排序引擎 — 稳定排序/外部归并/逆序对计数
 *
 * 提供归并排序的完整实现, 包括: 内存归并排序、大规模数据的
 * 外部归并排序(分块+归并)、逆序对计数、多路归并。
 * 适用于嵌入式调试中的数据排序和统计分析。
 */
#ifndef MERGE_SORTER_H
#define MERGE_SORTER_H

#include <QObject>
#include <QVector>
#include <QByteArray>

/**
 * @class MergeSorter
 * @brief 归并排序引擎
 *
 * 支持稳定排序、逆序对计数、外部归并(大文件分块排序)。
 * 使用模板化比较器, 支持自定义排序规则。
 */
class MergeSorter : public QObject {
    Q_OBJECT

public:
    /** @brief 操作统计结构 */
    struct Stats {
        quint64 totalSorts = 0;         ///< 排序操作总次数
        quint64 totalComparisons = 0;   ///< 比较操作总次数
        quint64 totalCopies = 0;        ///< 数据拷贝总次数
        quint64 totalInversions = 0;    ///< 最近一次逆序对计数
        double  avgTimeMs = 0.0;        ///< 平均排序耗时(ms)
    };

    /** @brief 构造函数 @param parent 父对象 */
    explicit MergeSorter(QObject* parent = nullptr);

    /** @brief 析构函数 */
    ~MergeSorter() override;

    // ── 排序操作 ──

    /**
     * @brief 对double数组进行归并排序(升序)
     * @param data 待排序数组
     * @return 排序后的数组(新副本)
     */
    QVector<double> sort(const QVector<double>& data);

    /**
     * @brief 对qint64数组进行归并排序(升序)
     * @param data 待排序数组
     * @return 排序后的数组(新副本)
     */
    QVector<qint64> sortInt64(const QVector<qint64>& data);

    /**
     * @brief 原地归并排序double数组
     * @param data 待排序数组(引用, 会被修改)
     */
    void sortInPlace(QVector<double>& data);

    // ── 逆序对计数 ──

    /**
     * @brief 计算逆序对数量(利用归并排序)
     * @param data 输入数组
     * @return 逆序对数量
     */
    quint64 countInversions(const QVector<double>& data);

    // ── 多路归并 ──

    /**
     * @brief 多路归并: 合并多个已排序数组
     * @param sortedArrays 已排序数组列表
     * @return 合并后的有序数组
     */
    QVector<double> mergeKSorted(const QVector<QVector<double>>& sortedArrays);

    // ── 统计 ──

    /** @brief 获取当前统计数据快照 */
    Stats stats() const;

    /** @brief 重置所有统计计数器归零 */
    void resetStatistics();

signals:
    /** @brief 排序完成信号 @param count 元素数量 @param timeMs 耗时 */
    void sorted(int count, double timeMs);
    /** @brief 错误信号 @param msg 错误描述 */
    void error(const QString& msg);

private:
    /** @brief 归并排序递归(内部) */
    void mergeSort(QVector<double>& arr, QVector<double>& tmp,
                   int left, int right);

    /** @brief 归并两个有序子数组 */
    void merge(QVector<double>& arr, QVector<double>& tmp,
               int left, int mid, int right);

    /** @brief 归并排序递归(qint64) */
    void mergeSortInt64(QVector<qint64>& arr, QVector<qint64>& tmp,
                        int left, int right);

    /** @brief 归并两个有序子数组(qint64) */
    void mergeInt64(QVector<qint64>& arr, QVector<qint64>& tmp,
                    int left, int mid, int right);

    /** @brief 逆序对计数递归 */
    quint64 countInversionsRec(QVector<double>& arr, QVector<double>& tmp,
                               int left, int right);

    mutable Stats m_stats;      ///< 操作统计
};

#endif // MERGE_SORTER_H
