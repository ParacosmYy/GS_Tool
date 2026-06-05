/**
 * @file SignalAligner.h
 * @brief 信号对齐器 — 互相关配准
 *
 * 功能: 基于互相关(NCC)的信号对齐，通过计算参考信号与目标信号
 *       的归一化互相关，找到最佳对齐偏移量。支持单信号对齐和
 *       多信号批量对齐。
 *
 * 协作: CrossCorrelator(互相关) / DataResampler(重采样)
 */
#ifndef SIGNALALIGNER_H
#define SIGNALALIGNER_H

#include <QObject>
#include <QVector>

/**
 * @brief 信号对齐器 — 互相关配准
 */
class SignalAligner : public QObject {
    Q_OBJECT

public:
    /** @brief 统计信息 */
    struct Stats {
        quint64 totalAlignments = 0;       ///< 累计对齐次数
        double  avgProcessingTimeMs = 0.0; ///< 平均处理时间(ms)
    };

    /** @brief 对齐结果 */
    struct AlignResult {
        QVector<double> aligned;   ///< 对齐后信号
        int offset = 0;            ///< 偏移量(样本)
        double correlation = 0.0;  ///< 最大互相关值
    };

    explicit SignalAligner(QObject* parent = nullptr);

    /** @brief 对齐单个信号到参考信号
     *  @param reference 参考信号
     *  @param signal 待对齐信号
     *  @return 对齐结果 */
    AlignResult align(const QVector<double>& reference,
                      const QVector<double>& signal);

    /** @brief 批量对齐多个信号到参考信号
     *  @param signals 待对齐信号列表
     *  @param reference 参考信号
     *  @return 对齐结果列表 */
    QVector<AlignResult> alignMultiple(
        const QVector<QVector<double>>& signalList,
        const QVector<double>& reference);

    /** @brief 设置最大搜索范围
     *  @param maxOffset 最大偏移样本数(0=自动) */
    void setMaxOffset(int maxOffset);

    /** @brief 获取统计 */
    const Stats& stats() const { return m_stats; }

    /** @brief 重置统计 */
    void resetStatistics();

signals:
    /** @brief 对齐完成 @param offset 偏移量 @param correlation 互相关值 */
    void alignmentCompleted(int offset, double correlation);

private:
    /** @brief 归一化互相关 @param ref 参考 @param sig 信号 @return (偏移, 相关值) */
    QPair<int, double> crossCorrelate(const QVector<double>& ref,
                                      const QVector<double>& sig) const;

    int m_maxOffset;     ///< 最大搜索偏移
    double m_timeSum;    ///< 处理时间累加器
    Stats  m_stats;      ///< 统计信息
};

#endif // SIGNALALIGNER_H
