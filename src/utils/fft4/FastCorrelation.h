/**
 * @file FastCorrelation.h
 * @brief 快速互相关 — 基于FFT的互相关/自相关计算
 *
 * 功能: 利用FFT实现O(NlogN)互相关计算，支持信号延迟检测。
 *       提供互相关、自相关和滞后搜索功能。
 *
 * 协作: FftEngine(频谱分析) / CrossCorrelator(时域直接法)
 */
#ifndef FASTCORRELATION_H
#define FASTCORRELATION_H

#include <QObject>
#include <QVector>

/**
 * @brief 快速互相关计算器
 */
class FastCorrelation : public QObject {
    Q_OBJECT

public:
    /** @brief 统计信息 */
    struct Stats {
        quint64 totalCorrelations = 0;    ///< 累计相关计算次数
        double  avgProcessingTimeMs = 0.0; ///< 平均处理时间(ms)
    };

    explicit FastCorrelation(QObject* parent = nullptr);

    /** @brief 计算两个信号的互相关
     *  @param signal1 第一路信号
     *  @param signal2 第二路信号
     *  @return 互相关结果 */
    QVector<double> correlate(const QVector<double>& signal1,
                              const QVector<double>& signal2);

    /** @brief 计算信号的自相关
     *  @param signal 输入信号
     *  @return 自相关结果 */
    QVector<double> autoCorrelate(const QVector<double>& signal);

    /** @brief 查找两个信号之间的滞后
     *  @param signal1 第一路信号
     *  @param signal2 第二路信号
     *  @return 滞后样本数(正=signal2滞后于signal1) */
    int findLag(const QVector<double>& signal1,
                const QVector<double>& signal2);

    /** @brief 获取统计 */
    const Stats& stats() const { return m_stats; }

    /** @brief 重置统计 */
    void resetStatistics();

signals:
    /** @brief 相关计算完成 @param lagCount 结果长度 */
    void correlationCompleted(int lagCount);

private:
    /** @brief 执行FFT(基2-Cooley-Tukey)
     *  @param data 复数数组(real,imag交错) @param n 长度 @param inverse 是否逆变换 */
    void fftImpl(QVector<double>& data, int n, bool inverse);

    /** @brief 补零到2的幂次 */
    int nextPowerOf2(int n) const;

    double m_timeSum;   ///< 处理时间累加器
    Stats  m_stats;     ///< 统计信息
};

#endif // FASTCORRELATION_H
