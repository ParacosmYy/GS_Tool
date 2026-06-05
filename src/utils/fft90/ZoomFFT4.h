#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief Zoom FFT频谱分析工具类
 *
 * 提供Zoom FFT细化和频谱分析功能，支持设置中心频率和带宽，
 * 可在指定频段内获得更高的频率分辨率。
 */
class ZoomFFT4 : public QObject {
    Q_OBJECT
public:
    /// 计算统计信息
    struct Stats {
        int totalComputations = 0;  ///< 总计算次数
        double avgProcessingTimeMs = 0.0; ///< 平均处理耗时(ms)
    };

    explicit ZoomFFT4(QObject* parent = nullptr);

    /** @brief 设置中心频率(Hz) */
    void setCenterFreq(double freqHz);

    /** @brief 设置分析带宽(Hz) */
    void setBandwidth(double bwHz);

    /** @brief 对输入信号执行Zoom FFT计算 */
    QVector<double> compute(const QVector<double>& input);

    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 计算完成信号，返回输出频谱点数 */
    void computationCompleted(int spectrumSize);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
    double m_centerFreq = 1000.0;
    double m_bandwidth = 500.0;
};
