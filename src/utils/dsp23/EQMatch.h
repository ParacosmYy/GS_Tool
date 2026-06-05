/**
 * @file EQMatch.h
 * @brief EQ匹配引擎 — 源到目标EQ曲线匹配/最小相位滤波器设计
 *
 * 功能: 计算从源频响到目标频响的匹配EQ曲线，设计最小相位滤波器，
 *       支持幅度匹配、平滑插值和增益限制。
 *
 * 协作: DigitalFilter(数字滤波器) / SpectrumAnalyzer(频谱分析)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief EQ匹配/滤波器传递函数引擎
 */
class EQMatch : public QObject {
    Q_OBJECT

public:
    /** @brief 匹配模式 */
    enum class MatchMode {
        MagnitudeOnly,     ///< 仅幅度匹配
        MagnitudePhase,    ///< 幅度+相位匹配
        MinimumPhase       ///< 最小相位设计
    };
    Q_ENUM(MatchMode)

    /** @brief EQ频段 */
    struct EQBand {
        double frequency = 0.0;     ///< 中心频率(Hz)
        double gain = 0.0;          ///< 增益(dB)
        double Q = 1.0;             ///< 品质因数
        double bandwidth = 1.0;     ///< 带宽(倍频程)
    };

    /** @brief 统计 */
    struct Stats {
        quint64 totalMatches = 0;           ///< 累计匹配次数
        quint64 totalBinsProcessed = 0;     ///< 累计处理频率bin数
        double  avgProcessingTimeMs = 0.0;  ///< 平均处理时间(ms)
        double  lastMatchError = 0.0;       ///< 最近匹配误差(dB)
    };

    explicit EQMatch(QObject* parent = nullptr);

    /** @brief 设置匹配模式 @param mode 匹配模式 */
    void setMatchMode(MatchMode mode);

    /** @brief 设置最大增益限制 @param maxGainDb 最大增益(dB) */
    void setMaxGain(double maxGainDb);

    /** @brief 设置平滑系数 @param smoothness 0~1(0=无平滑,1=最大平滑) */
    void setSmoothness(double smoothness);

    /** @brief 计算EQ匹配曲线 @param sourceMag 源幅度谱(dB) @param targetMag 目标幅度谱(dB) @param frequencies 频率轴 @return 匹配增益曲线(dB) */
    QVector<double> matchEQ(const QVector<double>& sourceMag,
                            const QVector<double>& targetMag,
                            const QVector<double>& frequencies);

    /** @brief 从增益曲线设计参数EQ频段 @param gains 增益曲线 @param frequencies 频率轴 @param numBands 频段数 @return EQ频段列表 */
    QList<EQBand> designParametricEQ(const QVector<double>& gains,
                                     const QVector<double>& frequencies,
                                     int numBands);

    /** @brief 应用最小相位变换 @param magnitude 线性幅度谱 @return 最小相位冲激响应 */
    QVector<double> minimumPhaseImpulse(const QVector<double>& magnitude);

    /** @brief 平滑增益曲线 @param gains 输入增益 @param frequencies 频率轴 @return 平滑后增益 */
    QVector<double> smoothGainCurve(const QVector<double>& gains,
                                    const QVector<double>& frequencies);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 匹配完成 @param matchError 匹配误差(dB) */
    void matchComplete(double matchError);

    /** @brief EQ频段设计完成 @param numBands 频段数 */
    void eqDesigned(int numBands);

private:
    QVector<double> computeCepstrum(const QVector<double>& magnitude);
    double findPeakGain(const QVector<double>& gains) const;

    MatchMode m_mode;               ///< 匹配模式
    double m_maxGain;               ///< 最大增益限制(dB)
    double m_smoothness;            ///< 平滑系数

    Stats m_stats;
    double m_timeSum = 0.0;         ///< 处理时间累加器
};
