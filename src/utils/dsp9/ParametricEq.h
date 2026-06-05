/**
 * @file ParametricEq.h
 * @brief 参数均衡器 — 支持Peaking/LowShelf/HighShelf/Notch四种滤波段
 *
 * 功能: 多段参数均衡器，每段可独立配置滤波器类型、中心频率、增益和Q值。
 *       基于双二阶(biquad) IIR滤波器实现，支持实时处理和批量处理。
 *
 * 协作: WaveformGenerator(信号源) / WaveformEngine(波形显示)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QList>

/**
 * @brief 参数均衡器 — 多段双二阶IIR滤波
 */
class ParametricEq : public QObject {
    Q_OBJECT

public:
    /** @brief 滤波器段类型 */
    enum class FilterType {
        Peaking,    ///< 峰值滤波(钟形)
        LowShelf,   ///< 低频搁架
        HighShelf,  ///< 高频搁架
        Notch       ///< 陷波(带阻)
    };
    Q_ENUM(FilterType)

    /** @brief 单个滤波段参数 */
    struct Band {
        FilterType type = FilterType::Peaking;  ///< 滤波器类型
        double frequency = 1000.0;              ///< 中心频率(Hz)
        double gainDb = 0.0;                    ///< 增益(dB)
        double q = 1.414;                       ///< 品质因数Q
        bool enabled = true;                    ///< 是否启用
    };

    /** @brief 双二阶系数 */
    struct BiquadCoeffs {
        double b0 = 1.0;    ///< 前馈系数b0
        double b1 = 0.0;    ///< 前馈系数b1
        double b2 = 0.0;    ///< 前馈系数b2
        double a1 = 0.0;    ///< 反馈系数a1
        double a2 = 0.0;    ///< 反馈系数a2
    };

    /** @brief 运行统计 */
    struct Stats {
        int totalFramesProcessed = 0;       ///< 累计处理帧数
        int totalSamplesProcessed = 0;      ///< 累计处理采样数
        int totalBandUpdates = 0;           ///< 累计频段参数更新次数
        double avgProcessingTimeMs = 0.0;   ///< 平均处理时间(ms)
    };

    explicit ParametricEq(QObject* parent = nullptr);

    /** @brief 设置采样率 @param rate 采样率(Hz) */
    void setSampleRate(double rate);

    /** @brief 添加滤波段 @param band 滤波段参数 @return 段索引 */
    int addBand(const Band& band);

    /** @brief 移除滤波段 @param index 段索引 */
    void removeBand(int index);

    /** @brief 更新滤波段参数 @param index 段索引 @param band 新参数 */
    void updateBand(int index, const Band& band);

    /** @brief 获取所有频段 @return 频段列表 */
    QList<Band> bands() const;

    /** @brief 处理单个采样点 @param sample 输入采样 @return 滤波后采样 */
    double processSample(double sample);

    /** @brief 批量处理数据 @param input 输入数据 @return 滤波后数据 */
    QVector<double> processBuffer(const QVector<double>& input);

    /** @brief 计算指定频段的频率响应 @param frequencies 频率数组 @param bandIndex 频段索引 @return 幅度响应(dB) */
    QVector<double> frequencyResponse(const QVector<double>& frequencies,
                                      int bandIndex) const;

    /** @brief 计算总频率响应 @param frequencies 频率数组 @return 总幅度响应(dB) */
    QVector<double> totalFrequencyResponse(const QVector<double>& frequencies) const;

    /** @brief 重置所有滤波器状态(清空延迟线) */
    void resetState();

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 频段参数已更新 @param index 频段索引 */
    void bandUpdated(int index);

    /** @brief 处理完成 @param sampleCount 采样数 */
    void processingComplete(int sampleCount);

private:
    /** @brief 根据Band参数计算双二阶系数 @param band 滤波段参数 @return 双二阶系数 */
    BiquadCoeffs computeCoefficients(const Band& band) const;

    /** @brief 双二阶滤波器状态 */
    struct BiquadState {
        double x1 = 0.0;   ///< 上一次输入
        double x2 = 0.0;   ///< 上上次输入
        double y1 = 0.0;   ///< 上一次输出
        double y2 = 0.0;   ///< 上上次输出
    };

    /** @brief 内部滤波段(包含系数和状态) */
    struct InternalBand {
        Band params;            ///< 段参数
        BiquadCoeffs coeffs;    ///< 计算后系数
        BiquadState state;      ///< 运行状态
    };

    double m_sampleRate = 44100.0;              ///< 采样率
    QList<InternalBand> m_bands;                ///< 滤波段列表

    Stats m_stats;                              ///< 运行统计
    double m_timeSum = 0.0;                     ///< 处理时间累加器
};
