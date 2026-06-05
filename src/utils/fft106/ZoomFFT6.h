#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief ZoomFFT细化频谱分析实现 (版本6)
 *
 * 提供频域细化分析功能，通过频移和降采样实现对指定频段的高分辨率分析。
 */
class ZoomFFT6 : public QObject {
    Q_OBJECT
public:
    /// 统计信息结构
    struct Stats {
        int totalZoomAnalyses = 0;      ///< 总细化分析次数
        double avgProcessingTimeMs = 0.0; ///< 平均处理耗时(ms)
        double maxFrequencyResolution = 0.0; ///< 最大频率分辨率(Hz)
    };

    explicit ZoomFFT6(QObject* parent = nullptr);
    Stats stats() const { return m_stats; }
    void resetStatistics();

    /**
     * @brief 执行ZoomFFT细化频谱分析
     * @param samples 输入时域采样数据
     * @param centerFreqHz 中心频率(Hz)
     * @param spanHz 分析带宽(Hz)
     * @return 细化后的频谱幅度
     */
    QVector<double> analyze(const QVector<double>& samples, double centerFreqHz, double spanHz);

    /**
     * @brief 设置采样率
     * @param sampleRateHz 采样率(Hz)
     */
    void setSampleRate(double sampleRateHz);

    /**
     * @brief 获取细化频谱的频率轴
     * @return 频率点序列(Hz)
     */
    QVector<double> frequencyAxis() const { return m_freqAxis; }

    /**
     * @brief 获取当前频率分辨率
     * @return 频率分辨率(Hz)
     */
    double frequencyResolution() const;

signals:
    /// 细化分析完成信号
    void zoomAnalysisCompleted(int binCount);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
    double m_sampleRate = 44100.0;
    QVector<double> m_freqAxis;
    int m_zoomFactor = 8;
};
