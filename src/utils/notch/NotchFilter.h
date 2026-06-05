/**
 * @file NotchFilter.h
 * @brief 陷波滤波器 — 工频干扰去除
 *
 * 功能: 基于二阶IIR结构的陷波(带阻)滤波器，用于去除特定频率
 *       的窄带干扰，如50/60Hz工频干扰。支持可调品质因数Q和
 *       采样率设置，可设计多个陷波频率。
 *
 * 协作: DigitalFilter(通用滤波) / Butterworth(低通/高通)
 */
#ifndef NOTCHFILTER_H
#define NOTCHFILTER_H

#include <QObject>
#include <QVector>

/**
 * @brief 陷波滤波器 — 工频干扰去除
 */
class NotchFilter : public QObject {
    Q_OBJECT

public:
    /** @brief 统计信息 */
    struct Stats {
        quint64 totalDesigns = 0;          ///< 累计设计次数
        quint64 totalApplications = 0;     ///< 累计应用次数
        double  avgProcessingTimeMs = 0.0; ///< 平均处理时间(ms)
    };

    explicit NotchFilter(QObject* parent = nullptr);

    /** @brief 设计陷波滤波器
     *  @param freq 陷波中心频率(Hz)
     *  @param qFactor 品质因数(越大带宽越窄)
     *  @param sampleRate 采样率(Hz) */
    void design(double freq, double qFactor, double sampleRate);

    /** @brief 应用陷波滤波器
     *  @param signal 输入信号
     *  @return 滤波后信号 */
    QVector<double> apply(const QVector<double>& signal);

    /** @brief 单步滤波(实时处理)
     *  @param sample 输入样本
     *  @return 滤波后样本 */
    double processSample(double sample);

    /** @brief 重置滤波器内部状态 */
    void resetState();

    /** @brief 获取中心频率 @return 中心频率(Hz) */
    double centerFrequency() const { return m_freq; }

    /** @brief 获取统计 */
    const Stats& stats() const { return m_stats; }

    /** @brief 重置统计 */
    void resetStatistics();

signals:
    /** @brief 滤波器设计完成 @param freq 中心频率 @param qFactor 品质因数 */
    void designCompleted(double freq, double qFactor);

    /** @brief 滤波应用完成 @param inputSize 输入信号长度 */
    void applicationCompleted(int inputSize);

private:
    double m_b0, m_b1, m_b2;   ///< 分子系数
    double m_a1, m_a2;         ///< 分母系数(a0=1)
    double m_x1, m_x2;        ///< 输入延迟线
    double m_y1, m_y2;        ///< 输出延迟线
    double m_freq;             ///< 中心频率
    double m_timeSum;          ///< 处理时间累加器
    Stats  m_stats;            ///< 统计信息
};

#endif // NOTCHFILTER_H
