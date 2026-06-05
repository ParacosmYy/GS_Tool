#pragma once
#include <QObject>
#include <QVector>

/**
 * @brief Chirp Z变换(CZT)实现
 *
 * 在单位圆上任意螺旋路径上计算z变换,提供比FFT更灵活的
 * 频率分辨率控制,适用于窄带频谱分析与雷达信号处理。
 */
class ChirpZTransform3 : public QObject {
    Q_OBJECT
public:
    /// 统计信息结构体
    struct Stats { int totalComputed = 0; double avgProcessingTimeMs = 0.0; };

    explicit ChirpZTransform3(QObject* parent = nullptr);

    /** @brief 设置螺旋比率参数 */
    void setRatio(double ratio);

    /** @brief 设置变换阶数 */
    void setOrder(int order);

    /** @brief 计算Chirp Z变换 */
    void compute(const QVector<double>& input);

    /** @brief 获取当前统计信息 */
    Stats stats() const { return m_stats; }

    /** @brief 重置统计数据 */
    void resetStatistics();

signals:
    /** @brief 计算完成信号 */
    void computationCompleted(int outputSize);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
    double m_ratio = 1.0;
    int m_order = 64;
};
