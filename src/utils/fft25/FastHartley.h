/**
 * @file FastHartley.h
 * @brief 快速Hartley变换(FHT) — 实数频谱分析
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 快速Hartley变换引擎
 * FHT与FFT等价但仅处理实数,无复数运算
 */
class FastHartley : public QObject
{
    Q_OBJECT

public:
    /** @brief 统计信息 */
    struct Stats {
        int totalTransforms = 0;          ///< 累计变换次数
        int totalInverseTransforms = 0;   ///< 累计逆变换次数
        int totalSamplesProcessed = 0;    ///< 累计处理采样数
        double avgProcessingTimeMs = 0.0;
    };

    explicit FastHartley(QObject* parent = nullptr);

    /** @brief 正向FHT @param data 输入数据(长度必须为2的幂) @return Hartley变换结果 */
    QVector<double> forward(const QVector<double>& data);

    /** @brief 逆向FHT @param spectrum Hartley谱 @return 重建信号 */
    QVector<double> inverse(const QVector<double>& spectrum);

    /** @brief 从Hartley谱计算功率谱 @param spectrum Hartley谱 @return 功率谱 */
    QVector<double> powerSpectrum(const QVector<double>& spectrum) const;

    /** @brief 从Hartley谱计算幅度和相位 @return (幅度,相位) */
    QPair<QVector<double>, QVector<double>> magnitudePhase(
        const QVector<double>& spectrum) const;

    /** @brief Hartley域卷积 @param h1 谱1 @param h2 谱2 @return 卷积谱 */
    QVector<double> convolve(const QVector<double>& h1,
                             const QVector<double>& h2) const;

    /** @brief Hartley域自相关 @param spectrum Hartley谱 @return 自相关 */
    QVector<double> autocorrelate(const QVector<double>& spectrum) const;

    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 变换完成 @param size 数据大小 */
    void transformCompleted(int size);

private:
    /** @brief 原地FHT @param data 数据(长度必须为2的幂) */
    void fhtInPlace(QVector<double>& data) const;

    Stats m_stats;
    double m_timeSum = 0.0;
};
