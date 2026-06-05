/**
 * @file Compressor.h
 * @brief 动态范围压缩器 — 攻击/释放包络/软硬膝/RMS+峰值检测
 *
 * 功能: 实现专业级动态范围压缩器，支持攻击/释放时间控制，
 *       软/硬膝函数切换，RMS和峰值检测模式，增益补偿，
 *       侧链输入，用于串口音频数据流的动态范围处理。
 *
 * 协作: WaveformGenerator(信号源) / DigitalFilter(预处理)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 动态范围压缩器 — 攻击释放包络/膝函数/增益补偿
 */
class Compressor : public QObject {
    Q_OBJECT

public:
    /** @brief 检测模式 */
    enum class DetectionMode {
        RMS,        ///< 均方根检测
        Peak,       ///< 峰值检测
        Hybrid      ///< 混合检测(RMS与峰值加权)
    };
    Q_ENUM(DetectionMode)

    /** @brief 膝函数类型 */
    enum class KneeType {
        Hard,       ///< 硬膝(阈值处突变)
        Soft        ///< 软膝(阈值附近平滑过渡)
    };
    Q_ENUM(KneeType)

    /** @brief 压缩器参数 */
    struct Parameters {
        double thresholdDb = -20.0;    ///< 阈值(dB)
        double ratio = 4.0;            ///< 压缩比(1:∞)
        double kneeDb = 6.0;           ///< 膝宽度(dB)
        double attackMs = 10.0;        ///< 攻击时间(ms)
        double releaseMs = 100.0;      ///< 释放时间(ms)
        double makeupGainDb = 0.0;     ///< 增益补偿(dB)
        double mix = 1.0;              ///< 干湿混合(0~1)
        DetectionMode detection = DetectionMode::RMS;
        KneeType kneeType = KneeType::Soft;
    };

    /** @brief 增益分析结果 */
    struct GainAnalysis {
        double inputLevelDb = 0.0;     ///< 输入电平(dB)
        double outputLevelDb = 0.0;    ///< 输出电平(dB)
        double gainReductionDb = 0.0;  ///< 增益衰减(dB)
        double envelopeDb = 0.0;       ///< 当前包络(dB)
    };

    /** @brief 统计信息 */
    struct Stats {
        quint64 totalSamplesProcessed = 0; ///< 累计处理样本数
        double  avgProcessingTimeMs = 0.0; ///< 平均处理时间(ms)
        double  peakGainReductionDb = 0.0; ///< 峰值增益衰减
        double  avgGainReductionDb = 0.0;  ///< 平均增益衰减
        quint64 totalSidechainTriggers = 0;///< 侧链触发次数
    };

    explicit Compressor(QObject* parent = nullptr);

    /** @brief 设置参数 @param params 压缩器参数 */
    void setParameters(const Parameters& params);

    /** @brief 获取当前参数 @return 参数 */
    const Parameters& parameters() const { return m_params; }

    /** @brief 设置采样率 @param rate 采样率(Hz) */
    void setSampleRate(double rate);

    /** @brief 处理单个样本 @param input 输入样本 @return 输出样本 */
    double processSample(double input);

    /** @brief 处理带侧链输入的样本 @param input 主输入 @param sidechain 侧链输入 @return 输出 */
    double processSampleSidechain(double input, double sidechain);

    /** @brief 批量处理 @param input 输入缓冲区 @return 输出缓冲区 */
    QVector<double> process(const QVector<double>& input);

    /** @brief 批量处理带侧链 @param input 主输入 @param sidechain 侧链 @return 输出 */
    QVector<double> processSidechain(const QVector<double>& input,
                                     const QVector<double>& sidechain);

    /** @brief 获取最近增益分析 @return 增益分析结果 */
    GainAnalysis lastGainAnalysis() const { return m_lastAnalysis; }

    /** @brief 计算静态传输特性 @param inputDb 输入电平(dB) @return 输出电平(dB) */
    double transferCharacteristic(double inputDb) const;

    /** @brief 重置内部状态(包络等) */
    void reset();

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 增益衰减变化 @param reductionDb 衰减量(dB) */
    void gainReductionChanged(double reductionDb);

    /** @brief 侧链触发 @param levelDb 侧链电平 */
    void sidechainTriggered(double levelDb);

private:
    double computeGainReduction(double inputDb);
    double detectLevel(const QVector<double>& window) const;
    void updateEnvelope(double targetDb);

    Parameters m_params;            ///< 压缩器参数
    double m_sampleRate;            ///< 采样率
    double m_envelopeDb;            ///< 当前包络电平(dB)
    double m_attackCoeff;           ///< 攻击系数
    double m_releaseCoeff;          ///< 释放系数
    QVector<double> m_rmsWindow;    ///< RMS检测窗口
    int m_rmsWindowIdx;             ///< 窗口索引

    GainAnalysis m_lastAnalysis;    ///< 最近分析结果
    double m_grSum;                 ///< 增益衰减累加器
    quint64 m_grCount;              ///< 增益衰减计数

    Stats m_stats;
    double m_timeSum = 0.0;         ///< 处理时间累加器
};
