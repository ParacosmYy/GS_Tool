/**
 * @file SignalGenerator.h
 * @brief 信号发生器 — 多种波形生成与调制
 *
 * 功能: 支持正弦/方波/三角/锯齿/噪声波形生成，可配置频率、
 *       幅度、相位、直流偏移，支持AM/FM调制，用于测试信号合成。
 *
 * 协作: WaveformEngine(波形显示) / DataPipeline(信号注入)
 */
#ifndef SIGNALGENERATOR_H
#define SIGNALGENERATOR_H

#include <QObject>
#include <QVector>

/**
 * @class SignalGenerator
 * @brief 多波形信号发生器
 */
class SignalGenerator : public QObject {
    Q_OBJECT

public:
    /** 波形类型 */
    enum class WaveformType {
        Sine,          ///< 正弦波
        Square,        ///< 方波
        Triangle,      ///< 三角波
        Sawtooth,      ///< 锯齿波
        Noise          ///< 白噪声
    };

    /** 调制类型 */
    enum class ModulationType {
        None,          ///< 无调制
        AM,            ///< 幅度调制
        FM             ///< 频率调制
    };

    /** 生成统计 */
    struct Stats {
        quint64 totalGenerated = 0;               ///< 总生成次数
        quint64 totalSamplesGenerated = 0;        ///< 总生成样本数
        double  avgProcessingTime = 0.0;          ///< 平均处理时间(ms)
    };

    /** 波形参数配置 */
    struct WaveformParams {
        double frequency = 1.0;                    ///< 频率(Hz)
        double amplitude = 1.0;                    ///< 幅度
        double phase = 0.0;                        ///< 初始相位(rad)
        double offset = 0.0;                       ///< 直流偏移
        double sampleRate = 1000.0;                ///< 采样率(Hz)
    };

    /** 调制参数 */
    struct ModulationParams {
        ModulationType type = ModulationType::None;///< 调制类型
        double modFrequency = 0.1;                 ///< 调制频率(Hz)
        double modDepth = 0.5;                     ///< 调制深度(0~1)
    };

    explicit SignalGenerator(QObject* parent = nullptr);

    /** @brief 设置波形类型 @param type 波形类型 */
    void setWaveformType(WaveformType type);

    /** @brief 设置波形参数 @param params 波形参数 */
    void setWaveformParams(const WaveformParams& params);

    /** @brief 设置调制参数 @param params 调制参数 */
    void setModulationParams(const ModulationParams& params);

    /** @brief 设置随机种子 @param seed 种子值 */
    void setSeed(quint32 seed);

    /** @brief 生成指定数量样本 @param sampleCount 样本数 @return 样本向量 */
    QVector<double> generate(int sampleCount);

    /** @brief 生成指定时长的信号 @param durationSec 时长(秒) @return 样本向量 */
    QVector<double> generateDuration(double durationSec);

    /** @brief 计算单点波形值 @param t 时间(s) @return 波形值 */
    double sampleAt(double t) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 信号生成完成 @param samples 样本数据 */
    void signalGenerated(const QVector<double>& samples);

    /** @brief 生成进度 @param progress 0.0~1.0 */
    void generationProgress(double progress);

private:
    double computeBase(double t) const;    ///< 计算基础波形值
    double applyModulation(double value, double t) const; ///< 应用调制

    WaveformType m_waveformType;           ///< 波形类型
    WaveformParams m_params;               ///< 波形参数
    ModulationParams m_modParams;          ///< 调制参数
    quint32 m_seed;                        ///< 随机种子
    bool m_seedSet;                        ///< 是否设置了种子

    Stats m_stats;
    double m_timeSum;                      ///< 处理时间累计
};

#endif // SIGNALGENERATOR_H
