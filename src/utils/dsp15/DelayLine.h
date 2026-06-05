/**
 * @file DelayLine.h
 * @brief 分数延迟线 — Lagrange/全通插值 + 环形缓冲区
 *
 * 功能: 实现高性能分数延迟线，支持Lagrange插值和全通插值两种
 *       分数延迟方法，环形缓冲区管理，反馈/前馈控制。
 *       适用于音频效果(合唱/混响/Flanger)、通信定时恢复、
 *       波束成形、物理建模合成。
 *
 * 协作: AdaptiveFilter2(自适应滤波) / Beamformer(波束成形)
 */
#pragma once

#include <QObject>
#include <QVector>

#include <vector>

/**
 * @brief 分数延迟线 — Lagrange/全通插值
 */
class DelayLine : public QObject {
    Q_OBJECT

public:
    /** @brief 插值方法 */
    enum InterpolationMode {
        Lagrange = 0,       ///< Lagrange多项式插值(高精度)
        Allpass = 1,        ///< 一阶全通插值(相位线性)
        Linear = 2          ///< 线性插值(低复杂度)
    };
    Q_ENUM(InterpolationMode)

    /** @brief 延迟线参数 */
    struct Parameters {
        double delay = 0.0;             ///< 当前延迟(采样点)
        double feedback = 0.0;          ///< 反馈增益[0,1)
        double feedforward = 0.0;       ///< 前馈增益
        double dryMix = 1.0;            ///< 干信号混合比
        double wetMix = 1.0;            ///< 湿信号混合比
        InterpolationMode mode = Lagrange; ///< 插值模式
        int lagrangeOrder = 5;          ///< Lagrange插值阶数(奇数)
    };

    /** @brief 操作统计 */
    struct Stats {
        quint64 totalSamplesProcessed = 0; ///< 累计处理采样数
        quint64 totalDelayChanges = 0;     ///< 累计延迟变更次数
        double  avgProcessingTimeMs = 0.0; ///< 平均处理耗时(ms)
    };

    /**
     * @brief 构造函数
     * @param maxDelay 最大延迟(采样点, 默认44100)
     * @param parent 父对象
     */
    explicit DelayLine(int maxDelay = 44100, QObject* parent = nullptr);

    /** @brief 析构函数 */
    ~DelayLine() override;

    // ── 参数控制 ──

    /**
     * @brief 设置延迟参数
     * @param params 延迟线参数
     */
    void setParameters(const Parameters& params);

    /**
     * @brief 获取当前参数
     * @return 参数
     */
    Parameters parameters() const;

    /**
     * @brief 设置延迟值(采样点)
     * @param delay 延迟值
     */
    void setDelay(double delay);

    /**
     * @brief 设置反馈增益
     * @param feedback 反馈增益[0,1)
     */
    void setFeedback(double feedback);

    // ── 信号处理 ──

    /**
     * @brief 处理单个采样点
     * @param input 输入采样
     * @return 延迟处理后输出
     */
    double processOne(double input);

    /**
     * @brief 处理采样缓冲区
     * @param input 输入缓冲区
     * @return 处理后缓冲区
     */
    QVector<double> process(const QVector<double>& input);

    /**
     * @brief 重置延迟线状态
     */
    void reset();

    // ── 查询 ──

    /**
     * @brief 获取环形缓冲区当前读位置的值
     * @return 延迟线输出(未混合)
     */
    double tapOutput() const;

    /**
     * @brief 获取环形缓冲区中指定偏移的采样
     * @param offset 偏移(0=当前写位置)
     * @return 采样值
     */
    double tapAt(int offset) const;

    // ── 统计 ──

    /** @brief 获取统计 */
    Stats stats() const;

    /** @brief 重置统计 */
    void resetStatistics();

signals:
    /** @brief 延迟变更 @param newDelay 新延迟值 */
    void delayChanged(double newDelay);

private:
    /**
     * @brief Lagrange插值
     * @param frac 分数部分[0,1)
     * @return 插值结果
     */
    double interpolateLagrange(double frac);

    /**
     * @brief 全通插值
     * @param frac 分数部分[0,1)
     * @return 插值结果
     */
    double interpolateAllpass(double frac);

    /**
     * @brief 线性插值
     * @param frac 分数部分[0,1)
     * @return 插值结果
     */
    double interpolateLinear(double frac);

    /**
     * @brief 从环形缓冲区读取带偏移的采样
     * @param delaySamples 延迟采样数(含小数)
     * @return 插值后的采样
     */
    double readInterpolated(double delaySamples);

    int m_maxDelay;                  ///< 最大延迟
    Parameters m_params;             ///< 当前参数

    std::vector<double> m_buffer;    ///< 环形缓冲区
    int m_writePos;                  ///< 写位置

    double m_allpassY1;              ///< 全通前一次输出
    double m_allpassX1;             ///< 全通前一次输入

    Stats m_stats;                   ///< 操作统计
    double m_timeSum = 0.0;          ///< 累计耗时
};
