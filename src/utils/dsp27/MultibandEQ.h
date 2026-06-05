/**
 * @file MultibandEQ.h
 * @brief 多频段均衡器 — 分频滤波/频段增益/交叉合成/参数Q
 *
 * 功能: 将信号分为多个频段，每个频段独立控制增益和Q值，
 *       支持Linkwitz-Riley分频、峰值/搁架滤波、交叉合成。
 *
 * 协作: DigitalFilter(滤波) / WaveformGenerator(信号发生)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QList>

/**
 * @brief 多频段均衡器 — 分频滤波/频段增益/交叉合成
 */
class MultibandEQ : public QObject {
    Q_OBJECT

public:
    /** @brief 滤波器类型 */
    enum class FilterType {
        Peak,       ///< 峰值滤波
        LowShelf,   ///< 低频搁架
        HighShelf   ///< 高频搁架
    };
    Q_ENUM(FilterType)

    /** @brief 频段参数 */
    struct BandParam {
        double frequency = 1000.0;  ///< 中心频率(Hz)
        double gain = 0.0;          ///< 增益(dB)
        double q = 1.0;             ///< Q值(品质因数)
        FilterType type = FilterType::Peak; ///< 滤波器类型
        bool enabled = true;        ///< 是否启用
    };

    /** @brief 统计 */
    struct Stats {
        quint64 totalSamplesProcessed = 0; ///< 累计处理采样数
        quint64 totalProcessCalls = 0;     ///< 累计处理调用次数
        double  avgProcessingTimeMs = 0.0; ///< 平均处理时间(ms)
    };

    explicit MultibandEQ(QObject* parent = nullptr);

    /** @brief 设置采样率 @param rate 采样率(Hz) */
    void setSampleRate(double rate);

    /** @brief 添加频段 @param param 频段参数 @return 频段索引 */
    int addBand(const BandParam& param);

    /** @brief 修改频段参数 @param index 频段索引 @param param 新参数 */
    void setBandParam(int index, const BandParam& param);

    /** @brief 移除频段 @param index 频段索引 */
    void removeBand(int index);

    /** @brief 处理音频块 @param input 输入采样 @return 输出采样 */
    QVector<double> process(const QVector<double>& input);

    /** @brief 计算频率响应 @param freqCount 频率点数 @return (频率, 增益dB)对 */
    QVector<QPair<double, double>> frequencyResponse(int freqCount = 512) const;

    /** @brief 获取当前频段数 @return 频段数 */
    int bandCount() const;

    /** @brief 获取频段参数 @param index 频段索引 @return 参数 */
    BandParam bandParam(int index) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 处理完成 @param sampleCount 采样数 */
    void processComplete(int sampleCount);

private:
    /** @brief 双二阶滤波器系数 */
    struct BiquadCoeffs {
        double b0 = 1.0, b1 = 0.0, b2 = 0.0;
        double a1 = 0.0, a2 = 0.0;
    };

    /** @brief 双二阶滤波器状态 */
    struct BiquadState {
        double x1 = 0.0, x2 = 0.0;
        double y1 = 0.0, y2 = 0.0;
    };

    BiquadCoeffs computeCoeffs(const BandParam& param) const;
    double processBiquad(double sample, const BiquadCoeffs& c, BiquadState& s) const;
    void updateAllCoeffs();

    double m_sampleRate = 44100.0;           ///< 采样率
    QList<BandParam> m_bands;                ///< 频段列表
    QList<BiquadCoeffs> m_coeffs;            ///< 滤波器系数
    mutable QList<BiquadState> m_states;     ///< 滤波器状态

    Stats m_stats;
    double m_timeSum = 0.0;
};
