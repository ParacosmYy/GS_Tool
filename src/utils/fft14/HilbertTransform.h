/**
 * @file HilbertTransform.h
 * @brief 离散Hilbert变换 — 解析信号计算/瞬时幅度/瞬时相位/瞬时频率
 *
 * 功能: 通过FFT实现离散Hilbert变换，构造解析信号 z(t)=x(t)+j*H{x(t)}，
 *       提取瞬时幅度(包络)、瞬时相位和瞬时频率。
 *       支持频率域滤波和边界校正。
 *
 * 协作: SpectrumAnalyzer(频谱分析) / DataSmoother(包络平滑)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 离散Hilbert变换引擎 — 解析信号/包络/瞬时频率
 */
class HilbertTransform : public QObject {
    Q_OBJECT

public:
    /** @brief 边界处理模式 */
    enum class BoundaryMode {
        None,           ///< 不做边界校正
        Reflect,        ///< 镜像反射边界
        Taper           ///< 余弦锥化边界
    };
    Q_ENUM(BoundaryMode)

    /** @brief 解析信号结果 */
    struct AnalyticResult {
        QVector<double> analyticReal;       ///< 解析信号实部(原信号)
        QVector<double> analyticImag;       ///< 解析信号虚部(Hilbert变换)
        QVector<double> envelope;           ///< 瞬时幅度(包络)
        QVector<double> instantaneousPhase; ///< 瞬时相位(rad)
        QVector<double> instantaneousFreq;  ///< 瞬时频率(归一化)
    };

    /** @brief 运行时统计信息 */
    struct Stats {
        int totalTransforms = 0;                ///< 累计变换次数
        int totalSamplesProcessed = 0;          ///< 累计处理采样数
        int totalFFTSizeUsed = 0;               ///< 累计FFT大小
        double avgProcessingTimeMs = 0.0;       ///< 平均处理耗时(ms)
    };

    /** @brief 构造函数 @param parent 父对象 */
    explicit HilbertTransform(QObject* parent = nullptr);

    /** @brief 设置边界处理模式 @param mode 模式 */
    void setBoundaryMode(BoundaryMode mode);

    /** @brief 计算解析信号 @param data 输入信号 @return 解析信号结果 */
    AnalyticResult compute(const QVector<double>& data);

    /** @brief 仅计算包络(幅度) @param data 输入信号 @return 包络 */
    QVector<double> envelope(const QVector<double>& data);

    /** @brief 仅计算瞬时相位 @param data 输入信号 @return 瞬时相位(rad) */
    QVector<double> instantaneousPhase(const QVector<double>& data);

    /** @brief 计算瞬时频率 @param data 输入信号 @param sampleRate 采样率 @return 瞬时频率(Hz) */
    QVector<double> instantaneousFrequency(
        const QVector<double>& data, double sampleRate);

    /** @brief 获取当前统计 @return 统计常量引用 */
    const Stats& stats() const { return m_stats; }

    /** @brief 重置统计计数器 */
    void resetStatistics();

signals:
    /** @brief 变换完成 @param samples 处理采样数 */
    void transformComplete(int samples);

private:
    /** @brief 计算下一个2的幂 */
    static int nextPowerOf2(int n);

    /** @brief FFT(基2,就地,原地比特反转) */
    void fftInPlace(QVector<double>& real, QVector<double>& imag, bool inverse);

    /** @brief 应用边界校正 @param data 输入数据 @return 校正后数据 */
    QVector<double> applyBoundary(const QVector<double>& data);

    BoundaryMode m_boundaryMode;     ///< 边界处理模式

    Stats m_stats;                   ///< 运行时统计
    double m_timeSum = 0.0;          ///< 累计耗时(ms)
};
