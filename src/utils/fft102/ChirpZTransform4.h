#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief Chirp Z变换(CZT)实现
 *
 * 在单位圆上任意弧段上进行等间隔采样的频谱分析，
 * 可实现比标准FFT更灵活的频率分辨率控制。
 */
class ChirpZTransform4 : public QObject {
    Q_OBJECT
public:
    /// 统计信息结构体
    struct Stats { int totalComputed = 0; double avgProcessingTimeMs = 0.0; };

    explicit ChirpZTransform4(QObject* parent = nullptr);

    /** @brief 设置螺旋比参数 */
    void setRatio(double ratio);

    /** @brief 设置变换阶数 */
    void setOrder(int order);

    /** @brief 计算Chirp Z变换 */
    QVector<QPair<double,double>> compute(const QVector<double>& input);

    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 计算完成信号 */
    void computationCompleted(int pointCount);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
    double m_ratio = 1.0;
    int m_order = 64;
};
