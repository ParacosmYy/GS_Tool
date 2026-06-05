#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief Zoom FFT 细化频谱分析
 *
 * 通过数字下变频和降采样实现指定频段的精细化频谱分析，
 * 在有限FFT点数下获得更高的频率分辨率。
 */
class ZoomFFT5 : public QObject {
    Q_OBJECT
public:
    /// 统计信息结构体
    struct Stats {
        int totalComputations = 0;   ///< 已完成计算次数
        double avgProcessingTimeMs = 0.0; ///< 平均处理耗时(ms)
    };

    explicit ZoomFFT5(QObject* parent = nullptr);

    /** @brief 设置中心频率(Hz) */
    void setCenterFreq(double freq);
    /** @brief 设置分析带宽(Hz) */
    void setBandwidth(double bw);
    /** @brief 计算Zoom FFT频谱 */
    QVector<QPair<double, double>> compute(const QVector<double>& samples);

    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 计算完成，返回频谱点数 */
    void computationCompleted(int binCount);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
    double m_centerFreq = 1000.0;
    double m_bandwidth = 500.0;
};
