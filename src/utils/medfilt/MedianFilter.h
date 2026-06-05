/**
 * @file MedianFilter.h
 * @brief 中值滤波器 — 脉冲噪声去除
 *
 * 功能: 基于滑动窗口的中值滤波，有效去除脉冲噪声(椒盐噪声)
 *       同时保留信号边缘。支持一维信号和二维矩阵(图像)滤波。
 *       使用快速中值算法优化计算效率。
 *
 * 协作: SavitzkyGolay(多项式平滑) / DigitalFilter(一般滤波)
 */
#ifndef MEDIANFILTER_H
#define MEDIANFILTER_H

#include <QObject>
#include <QVector>

/**
 * @brief 中值滤波器 — 脉冲噪声去除
 */
class MedianFilter : public QObject {
    Q_OBJECT

public:
    /** @brief 统计信息 */
    struct Stats {
        quint64 totalApplications = 0;     ///< 累计应用次数
        double  avgProcessingTimeMs = 0.0; ///< 平均处理时间(ms)
    };

    explicit MedianFilter(QObject* parent = nullptr);

    /** @brief 设置窗口大小
     *  @param size 窗口大小(必须为奇数, >= 3) */
    void setWindowSize(int size);

    /** @brief 获取窗口大小 @return 窗口大小 */
    int windowSize() const { return m_windowSize; }

    /** @brief 一维中值滤波
     *  @param signal 输入信号
     *  @return 滤波后信号 */
    QVector<double> apply(const QVector<double>& signal);

    /** @brief 二维中值滤波(矩阵)
     *  @param matrix 输入矩阵(行优先)
     *  @param rows 行数
     *  @param cols 列数
     *  @return 滤波后矩阵 */
    QVector<double> apply2d(const QVector<double>& matrix,
                            int rows, int cols);

    /** @brief 获取统计 */
    const Stats& stats() const { return m_stats; }

    /** @brief 重置统计 */
    void resetStatistics();

signals:
    /** @brief 滤波应用完成 @param outputSize 输出信号长度 */
    void applicationCompleted(int outputSize);

private:
    /** @brief 快速中值选择 @param data 数据 @param n 长度 @param k 第k小 @return 结果 */
    static double quickSelect(QVector<double>& data, int k);

    int m_windowSize;    ///< 窗口大小
    double m_timeSum;    ///< 处理时间累加器
    Stats  m_stats;      ///< 统计信息
};

#endif // MEDIANFILTER_H
