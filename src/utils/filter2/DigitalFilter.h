/**
 * @file DigitalFilter.h
 * @brief 数字滤波器引擎 — 低通/高通/带通/带阻/中值/移动平均
 *
 * 功能: 6种数字滤波器，支持实时流式滤波和批量滤波，
 *       可调截止频率/阶数/窗口大小。
 *
 * 协作: WaveformFilter(DSP滤波) / DataTransformer(预处理)
 */
#ifndef DIGITALFILTER_H
#define DIGITALFILTER_H

#include <QObject>
#include <QVector>

/**
 * @brief 数字滤波器引擎 — 多类型实时滤波
 */
class DigitalFilter : public QObject {
    Q_OBJECT

public:
    /** @brief 滤波器类型 */
    enum class FilterType {
        LowPass,        ///< 低通滤波器
        HighPass,       ///< 高通滤波器
        BandPass,       ///< 带通滤波器
        BandStop,       ///< 带阻滤波器
        Median,         ///< 中值滤波器
        MovingAverage   ///< 移动平均滤波器
    };
    Q_ENUM(FilterType)

    /** @brief 统计 */
    struct Stats {
        quint64 totalSamplesProcessed = 0; ///< 累计处理采样数
        quint64 totalFiltersApplied = 0;   ///< 累计应用滤波次数
        double  averageLatencyUs = 0.0;    ///< 平均滤波延迟(us)
        double  peakInputValue = 0.0;      ///< 峰值输入
        double  peakOutputValue = 0.0;     ///< 峰值输出
    };

    explicit DigitalFilter(QObject* parent = nullptr);

    /** @brief 设置滤波器类型 @param type 类型 */
    void setFilterType(FilterType type);

    /** @brief 设置截止频率(归一化0-0.5) @param freq 截止频率 */
    void setCutoffFrequency(double freq);

    /** @brief 设置窗口大小(中值/移动平均) @param size 窗口大小 */
    void setWindowSize(int size);

    /** @brief 滤波单个采样值 @param input 输入值 @return 滤波后值 */
    double process(double input);

    /** @brief 批量滤波 @param data 输入数据 @return 滤波后数据 */
    QVector<double> processBatch(const QVector<double>& data);

    /** @brief 重置滤波器状态 */
    void reset();

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 滤波完成 @param count 采样数 */
    void filterApplied(int count);

private:
    double processLowPass(double input);
    double processHighPass(double input);
    double processMedian(double input);
    double processMovingAverage(double input);

    FilterType m_type;              ///< 滤波器类型
    double m_cutoffFreq;            ///< 截止频率
    int m_windowSize;               ///< 窗口大小

    /* IIR状态 */
    double m_x1, m_x2;             ///< 输入延迟
    double m_y1, m_y2;             ///< 输出延迟
    double m_a0, m_a1, m_a2;       ///< 分母系数
    double m_b0, m_b1, m_b2;       ///< 分子系数

    QVector<double> m_window;      ///< 滑动窗口
    double m_windowSum;            ///< 窗口累加和

    void updateCoefficients();

    Stats m_stats;
    double m_latencySum;           ///< 延迟累加器
};

#endif // DIGITALFILTER_H
