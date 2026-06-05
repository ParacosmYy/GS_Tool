/**
 * @file Beamformer.h
 * @brief 延迟-求和波束成形器 — 传感器阵列信号增强
 *
 * 功能: 实现基本的延迟-求和(delay-and-sum)波束成形算法，
 *       通过调整各传感器通道的时延和权重实现空间滤波，
 *       增强特定方向的信号、抑制其他方向干扰。
 *       适用于麦克风阵列、天线阵列、超声波成像等场景。
 *
 * 协作: SpectrumAnalyzer(频域分析) / DigitalFilter(预处理滤波)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QElapsedTimer>

/**
 * @brief 延迟-求和波束成形器
 *
 * 假设均匀线性阵列(ULA)，传感器间距spacing(m)，
 * 波速使用声速(343 m/s)或可通过接口自定义。
 */
class Beamformer : public QObject {
    Q_OBJECT

public:
    /** @brief 统计信息 */
    struct Stats {
        quint64 totalProcessed = 0;          ///< 累计处理帧数
        double  avgProcessingTimeMs = 0.0;   ///< 平均处理耗时(ms)
    };

    /**
     * @brief 构造函数
     * @param numSensors 传感器数量(通道数)
     * @param spacing    传感器间距(米)
     * @param sampleRate 采样率(Hz)
     * @param parent     父对象
     */
    explicit Beamformer(int numSensors = 4,
                        double spacing = 0.04,
                        double sampleRate = 44100.0,
                        QObject* parent = nullptr);

    /**
     * @brief 设置波束指向角度
     * @param angle 角度(度, 0=正前方, 正值=右偏)
     */
    void setSteeringAngle(double angle);

    /**
     * @brief 处理多通道信号帧
     * @param inputSignals inputSignals[channel][sample], 每通道一段采样
     * @return 波束成形后的单通道信号
     */
    QVector<double> process(const QVector<QVector<double>>& inputSignals);

    /**
     * @brief 设置各通道权重
     * @param weights 权重向量(长度=传感器数)，默认均匀权重
     */
    void setWeights(const QVector<double>& weights);

    /**
     * @brief 设置波传播速度
     * @param speed 波速(m/s), 声速343, 光速3e8等
     */
    void setWaveSpeed(double speed);

    /** @brief 获取当前波束角度(度) */
    double steeringAngle() const { return m_steeringAngle; }

    /** @brief 获取统计信息 */
    const Stats& stats() const { return m_stats; }

    /** @brief 重置统计信息 */
    void resetStatistics();

signals:
    /** @brief 处理完成信号 @param sampleCount 输出采样数 */
    void processingComplete(int sampleCount);

private:
    /**
     * @brief 计算各传感器相对参考传感器的时延(秒)
     * @return 时延数组(长度=numSensors)
     */
    QVector<double> computeDelays() const;

    /**
     * @brief sinc插值获取分数延迟采样值
     * @param signal 信号缓冲
     * @param delaySamples 延迟采样数(可为小数)
     * @return 插值结果
     */
    double sincInterpolate(const QVector<double>& signal,
                           double delaySamples) const;

    int     m_numSensors;       ///< 传感器数量
    double  m_spacing;          ///< 传感器间距(m)
    double  m_sampleRate;       ///< 采样率(Hz)
    double  m_waveSpeed;        ///< 波传播速度(m/s)
    double  m_steeringAngle;    ///< 波束指向角度(弧度)

    QVector<double> m_weights;  ///< 各通道权重

    QElapsedTimer m_timer;      ///< 计时器
    double  m_timeSum;          ///< 累计耗时
    Stats   m_stats;            ///< 统计信息
};
