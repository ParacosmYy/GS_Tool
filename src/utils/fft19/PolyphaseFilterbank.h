/**
 * @file PolyphaseFilterbank.h
 * @brief 多相滤波器组 — 均匀DFT滤波器组/原型滤波器/分析合成
 *
 * 功能: 实现M通道均匀DFT多相滤波器组，支持原型低通滤波器设计，
 *       多相分解，分析与合成滤波，用于串口数据的频带分割、
 *       子带处理和完美重构。
 *
 * 协作: SpectrumAnalyzer(频谱分析) / DigitalFilter(滤波)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 多相滤波器组 — 均匀DFT分析/合成滤波器组
 */
class PolyphaseFilterbank : public QObject {
    Q_OBJECT

public:
    /** @brief 原型滤波器类型 */
    enum class PrototypeType {
        KaiserWindow,       ///< Kaiser窗设计
        SincWindow,         ///< 截断sinc设计
        CosineModulated     ///< 余弦调制设计
    };
    Q_ENUM(PrototypeType)

    /** @brief 统计信息 */
    struct Stats {
        quint64 totalAnalyses = 0;          ///< 累计分析次数
        quint64 totalSyntheses = 0;         ///< 累计合成次数
        quint64 totalSamplesProcessed = 0;  ///< 累计处理样本数
        double  avgProcessingTimeMs = 0.0;  ///< 平均处理时间(ms)
        double  reconstructionError = 0.0;  ///< 重构误差
    };

    explicit PolyphaseFilterbank(QObject* parent = nullptr);

    /** @brief 设置通道数 @param M 通道数(2的幂) */
    void setChannelCount(int M);

    /** @brief 设置原型滤波器抽头数(每相) @param L 每相抽头数 */
    void setTapsPerPhase(int L);

    /** @brief 设置原型滤波器类型 @param type 滤波器类型 */
    void setPrototypeType(PrototypeType type);

    /** @brief 设计原型滤波器 @return 原型滤波器系数 */
    QVector<double> designPrototype();

    /** @brief 多相分解 @param prototype 原型滤波器 @return 多相矩阵[M][L] */
    QVector<QVector<double>> polyphaseDecompose(const QVector<double>& prototype);

    /** @brief 分析滤波器组(时域→子带) @param input 输入样本块 @return 子带信号[M个通道] */
    QVector<QVector<double>> analyze(const QVector<double>& input);

    /** @brief 合成滤波器组(子带→时域) @param subbands 子带信号 @return 重构输出 */
    QVector<double> synthesize(const QVector<QVector<double>>& subbands);

    /** @brief 完整分析-合成处理 @param input 输入 @return 重构输出 */
    QVector<double> processAnalysisSynthesis(const QVector<double>& input);

    /** @brief 获取子带中心频率 @return 频率数组(Hz) */
    QVector<double> subbandCenterFrequencies() const;

    /** @brief 计算重构误差 @param original 原始信号 @param reconstructed 重构信号 @return MSE */
    double reconstructionError(const QVector<double>& original,
                               const QVector<double>& reconstructed) const;

    /** @brief 获取原型滤波器系数 @return 系数 */
    const QVector<double>& prototypeFilter() const { return m_prototype; }

    /** @brief 获取通道数 @return M */
    int channelCount() const { return m_M; }

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 分析完成 @param channels 通道数 @param samplesPerChannel 每通道样本数 */
    void analysisComplete(int channels, int samplesPerChannel);

    /** @brief 合成完成 @param outputSamples 输出样本数 @param error 重构误差 */
    void synthesisComplete(int outputSamples, double error);

private:
    void computeDFT(QVector<double>& real, QVector<double>& imag);
    void computeIDFT(QVector<double>& real, QVector<double>& imag);
    double besselI0(double x) const;
    void generateKaiserWindow(int N, double beta, QVector<double>& window);
    void generateSincFilter(int N, double cutoff, QVector<double>& filter);

    int m_M;                       ///< 通道数
    int m_L;                       ///< 每相抽头数
    PrototypeType m_protoType;     ///< 原型滤波器类型
    double m_sampleRate;           ///< 采样率

    QVector<double> m_prototype;             ///< 原型滤波器系数
    QVector<QVector<double>> m_polyMatrix;   ///< 多相矩阵
    QVector<QVector<double>> m_stateMatrix;  ///< 状态缓冲区

    Stats m_stats;
    double m_timeSum = 0.0;        ///< 处理时间累加器
};
