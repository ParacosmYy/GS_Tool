/**
 * @file Oscillator.h
 * @brief 多波形振荡器 — BLEP抗混叠 + 调频/调幅合成
 *
 * 功能: 实现多波形振荡器，支持正弦/方波/锯齿/三角波生成，
 *       BLEP(Band-Limited stEP)抗混叠合成，频率/幅度调制，
 *       相位累加器，适用于音频合成、信号测试、通信基带生成。
 *
 * 协作: SignalGeneratorWidget(信号生成UI) / DelayLine(延迟效果)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 多波形振荡器 — BLEP抗混叠合成
 */
class Oscillator : public QObject {
    Q_OBJECT

public:
    /** @brief 波形类型 */
    enum Waveform {
        Sine = 0,           ///< 正弦波
        Square = 1,         ///< 方波
        Sawtooth = 2,       ///< 锯齿波
        Triangle = 3,       ///< 三角波
        Pulse = 4           ///< 脉冲波(可调占空比)
    };
    Q_ENUM(Waveform)

    /** @brief 振荡器参数 */
    struct Parameters {
        Waveform waveform = Sine;         ///< 波形类型
        double frequency = 440.0;         ///< 频率(Hz)
        double amplitude = 1.0;           ///< 振幅[0,1]
        double sampleRate = 44100.0;      ///< 采样率(Hz)
        double phase = 0.0;              ///< 初始相位[0,1)
        double pulseWidth = 0.5;          ///< 脉冲宽度(仅Pulse波)
        bool enableAntiAliasing = true;   ///< 是否启用BLEP抗混叠
        int blepPoints = 4;               ///< BLEP积分点数
        bool enableFM = false;            ///< 是否启用频率调制
        double fmDepth = 0.0;             ///< FM调制深度(Hz)
        double fmRate = 0.0;              ///< FM调制速率(Hz)
        bool enableAM = false;            ///< 是否启用幅度调制
        double amDepth = 0.0;             ///< AM调制深度[0,1]
        double amRate = 0.0;              ///< AM调制速率(Hz)
    };

    /** @brief 操作统计 */
    struct Stats {
        quint64 totalSamplesGenerated = 0; ///< 累计生成采样数
        quint64 totalFrequencyChanges = 0; ///< 累计频率变更次数
        quint64 totalWaveformChanges = 0;  ///< 累计波形变更次数
        double  avgProcessingTimeMs = 0.0; ///< 平均处理耗时(ms)
    };

    /**
     * @brief 构造函数
     * @param parent 父对象
     */
    explicit Oscillator(QObject* parent = nullptr);

    /** @brief 析构函数 */
    ~Oscillator() override;

    // ── 参数控制 ──

    /** @brief 设置振荡器参数 @param params 参数 */
    void setParameters(const Parameters& params);

    /** @brief 获取当前参数 @return 参数 */
    Parameters parameters() const;

    /** @brief 设置频率 @param freq 频率(Hz) */
    void setFrequency(double freq);

    /** @brief 设置振幅 @param amp 振幅[0,1] */
    void setAmplitude(double amp);

    /** @brief 设置波形类型 @param wf 波形 */
    void setWaveform(Waveform wf);

    // ── 信号生成 ──

    /**
     * @brief 生成单个采样点
     * @return 采样值[-1,1]
     */
    double tick();

    /**
     * @brief 生成指定数量的采样点
     * @param numSamples 采样数
     * @return 采样缓冲区
     */
    QVector<double> generate(int numSamples);

    /**
     * @brief 使用频率调制生成采样
     * @param numSamples 采样数
     * @param modSignal 调制信号(与输出等长)
     * @return 采样缓冲区
     */
    QVector<double> generateWithFM(int numSamples,
                                   const QVector<double>& modSignal);

    // ── 状态控制 ──

    /** @brief 重置相位累加器 */
    void resetPhase();

    /** @brief 设置当前相位 @param phase 相位[0,1) */
    void setPhase(double phase);

    /** @brief 获取当前相位 @return 相位[0,1) */
    double currentPhase() const;

    // ── 统计 ──

    /** @brief 获取统计 @return 统计 */
    Stats stats() const;

    /** @brief 重置统计 */
    void resetStatistics();

signals:
    /** @brief 频率变更 @param newFreq 新频率 */
    void frequencyChanged(double newFreq);

    /** @brief 波形变更 @param newWaveform 新波形 */
    void waveformChanged(int newWaveform);

    /** @brief 生成完成 @param numSamples 采样数 */
    void generationCompleted(int numSamples);

private:
    /**
     * @brief 原始波形采样(无抗混叠)
     * @param phase 相位[0,1)
     * @return 采样值
     */
    double rawSample(double phase) const;

    /**
     * @brief BLEP修正(方波)
     * @param phase 当前相位
     * @param inc 相位增量
     * @return BLEP修正量
     */
    double blepSquare(double phase, double inc) const;

    /**
     * @brief BLEP修正(锯齿波)
     * @param phase 当前相位
     * @param inc 相位增量
     * @return BLEP修正量
     */
    double blepSaw(double phase, double inc) const;

    /**
     * @brief BLEP积分(近似sinc积分)
     * @param t 归一化时间
     * @return 积分值
     */
    double blepIntegral(double t) const;

    /** @brief 更新相位增量 */
    void updatePhaseInc();

    Parameters m_params;                 ///< 振荡器参数
    double m_phaseAccum;                 ///< 相位累加器[0,1)
    double m_phaseInc;                   ///< 相位增量

    /* BLEP延迟线(存储上一次跳变的残余) */
    static constexpr int BLEP_HISTORY = 8;
    double m_blepHistorySaw[BLEP_HISTORY];   ///< 锯齿BLEP历史
    double m_blepHistorySq[BLEP_HISTORY];    ///< 方波BLEP历史

    Stats m_stats;                       ///< 操作统计
    double m_timeSum = 0.0;              ///< 累计耗时
};
