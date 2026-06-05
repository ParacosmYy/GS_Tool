/**
 * @file CicFilter.h
 * @brief CIC(级联积分梳状)滤波器 — 多速率信号处理
 *
 * 功能: 实现CIC抽取/插值滤波器，适用于多速率信号处理中的
 *       大比率采样率转换。无需乘法器，资源效率高。
 *
 * 协作: BesselFilter / ButterworthFilter / Decimator
 */
#ifndef CICFILTER_H
#define CICFILTER_H

#include <QObject>
#include <QVector>

/**
 * @brief CIC滤波器 — 积分梳状抽取/插值
 */
class CicFilter : public QObject {
    Q_OBJECT

public:
    /** @brief 统计信息 */
    struct Stats {
        quint64 totalSamples = 0;       ///< 累计处理样本数
        double avgProcessingTimeMs = 0.0; ///< 平均处理时间(ms)
    };

    explicit CicFilter(QObject* parent = nullptr);

    /** @brief 配置CIC滤波器
     *  @param order 滤波器阶数(积分器/梳状器对数)
     *  @param rate 变换比率(R)
     *  @param diffDelay 微分延迟(M，通常为1或2) */
    void configure(int order, int rate, int diffDelay = 1);

    /** @brief CIC抽取(降采样)
     *  @param input 输入样本
     *  @return 抽取后样本 */
    QVector<double> decimate(const QVector<double>& input);

    /** @brief CIC插值(升采样)
     *  @param input 输入样本
     *  @return 插值后样本 */
    QVector<double> interpolate(const QVector<double>& input);

    /** @brief 计算CIC幅度响应
     *  @param freqs 归一化频率数组(0~0.5)
     *  @return 幅度响应(dB) */
    QVector<double> frequencyResponse(const QVector<double>& freqs) const;

    /** @brief 获取统计 */
    const Stats& stats() const { return m_stats; }

    /** @brief 重置统计 */
    void resetStatistics();

    /** @brief 重置滤波器状态 */
    void reset();

signals:
    /** @brief 处理完成 @param inputSize 输入样本数 @param outputSize 输出样本数 */
    void processingCompleted(int inputSize, int outputSize);

private:
    int m_order;        ///< 阶数
    int m_rate;         ///< 变换比率
    int m_diffDelay;    ///< 微分延迟

    QVector<double> m_integrators;  ///< 积分器状态
    QVector<double> m_combRegs;     ///< 梳状器延迟寄存器

    double m_timeSum;   ///< 处理时间累加器
    Stats m_stats;      ///< 统计信息
};

#endif // CICFILTER_H
