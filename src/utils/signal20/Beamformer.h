/**
 * @file Beamformer.h
 * @brief 波束形成器 — 延迟求和/MVDR/Frost自适应/导向矢量
 *
 * 功能: 实现多种波束形成算法，包括延迟求和(DAS)、
 *       最小方差无畸变响应(MVDR)、Frost自适应波束形成，
 *       导向矢量计算和阵列增益分析，用于多通道信号的
 *       空间滤波和干扰抑制。
 *
 * 协作: SpectrumAnalyzer(频谱) / DigitalFilter(预滤波)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 波束形成器 — DAS/MVDR/Frost自适应/阵列增益
 */
class Beamformer : public QObject {
    Q_OBJECT

public:
    /** @brief 波束形成算法 */
    enum class Algorithm {
        DelayAndSum,    ///< 延迟求和(固定权重)
        MVDR,           ///< 最小方差无畸变响应(Capon)
        Frost           ///< Frost自适应波束形成
    };
    Q_ENUM(Algorithm)

    /** @brief 阵列配置 */
    struct ArrayConfig {
        int elementCount = 4;           ///< 阵元数
        double spacing = 0.5;           ///< 阵元间距(波长归一化)
        double speedOfSound = 343.0;    ///< 声速(m/s)
        QVector<double> positions;      ///< 阵元位置(可选自定义)
    };

    /** @brief 波束形成输出 */
    struct BeamOutput {
        QVector<double> output;             ///< 输出信号
        double snrImprovementDb = 0.0;      ///< SNR提升(dB)
        double arrayGainDb = 0.0;           ///< 阵列增益(dB)
        double whiteNoiseGainDb = 0.0;      ///< 白噪声增益(dB)
    };

    /** @brief 统计信息 */
    struct Stats {
        quint64 totalProcessed = 0;         ///< 累计处理样本数
        double  avgProcessingTimeMs = 0.0;  ///< 平均处理时间(ms)
        double  peakArrayGainDb = 0.0;      ///< 峰值阵列增益
        quint64 totalSteeringUpdates = 0;   ///< 导向矢量更新次数
    };

    explicit Beamformer(QObject* parent = nullptr);

    /** @brief 设置阵列配置 @param config 阵列参数 */
    void setArrayConfig(const ArrayConfig& config);

    /** @brief 设置波束形成算法 @param algorithm 算法 */
    void setAlgorithm(Algorithm algorithm);

    /** @brief 设置采样率 @param rate 采样率(Hz) */
    void setSampleRate(double rate);

    /** @brief 设置波束指向角 @param azimuthDeg 方位角(度) @param elevationDeg 仰角(度) */
    void setSteeringDirection(double azimuthDeg, double elevationDeg = 0.0);

    /** @brief 计算导向矢量 @param frequencyHz 频率 @return 导向矢量(复数实部/虚部交错) */
    QVector<double> steeringVector(double frequencyHz) const;

    /** @brief 处理多通道数据 @param input [通道×样本] @return 波束形成输出 */
    BeamOutput process(const QVector<QVector<double>>& input);

    /** @brief 延迟求和波束形成 @param input 多通道输入 @param weights 权重(空则均匀) @return 输出信号 */
    QVector<double> delayAndSum(const QVector<QVector<double>>& input,
                                const QVector<double>& weights = {});

    /** @brief MVDR波束形成 @param input 多通道输入 @return 输出信号 */
    QVector<double> mvdr(const QVector<QVector<double>>& input);

    /** @brief Frost自适应波束形成 @param input 多通道输入 @param stepSize 步长 @param constraintLambda 约束参数 @return 输出信号 */
    QVector<double> frost(const QVector<QVector<double>>& input,
                          double stepSize = 0.01,
                          double constraintLambda = 1.0);

    /** @brief 计算阵列增益 @param input 多通道输入 @param output 波束输出 @return 增益(dB) */
    double computeArrayGain(const QVector<QVector<double>>& input,
                            const QVector<double>& output) const;

    /** @brief 计算波束图 @param frequencyHz 频率 @param angleResolutionDeg 角度分辨率 @return (角度列表, 增益列表) */
    QPair<QVector<double>, QVector<double>> beamPattern(
        double frequencyHz, double angleResolutionDeg = 1.0) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 处理完成 @param samples 输出样本数 @param gainDb 阵列增益 */
    void processingComplete(int samples, double gainDb);

    /** @brief 导向矢量更新 @param azimuthDeg 方位角 */
    void steeringUpdated(double azimuthDeg);

private:
    double computeDelay(int elementIdx) const;
    QVector<QVector<double>> estimateCovariance(
        const QVector<QVector<double>>& input) const;
    void invertMatrix2x2(const QVector<QVector<double>>& M,
                         QVector<QVector<double>>& inv) const;
    QVector<QVector<double>> invertMatrix(
        const QVector<QVector<double>>& M) const;

    Algorithm m_algorithm;          ///< 当前算法
    ArrayConfig m_config;           ///< 阵列配置
    double m_sampleRate;            ///< 采样率
    double m_azimuth;               ///< 方位角(弧度)
    double m_elevation;             ///< 仰角(弧度)

    QVector<double> m_frostWeights; ///< Frost自适应权重

    Stats m_stats;
    double m_timeSum = 0.0;         ///< 处理时间累加器
};
