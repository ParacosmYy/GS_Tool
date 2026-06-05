/**
 * @file LinearPredictiveCoder.h
 * @brief 线性预测编码器 — LPC分析/合成
 *
 * 功能: 基于自相关法求解LPC系数，支持Levinson-Durbin递推，
 *       统计分析次数/阶数/残差能量。
 */
#ifndef LINEARPREDICTIVECODER_H
#define LINEARPREDICTIVECODER_H

#include <QObject>
#include <QVector>

class LinearPredictiveCoder : public QObject {
    Q_OBJECT
public:
    /** LPC分析结果 */
    struct LpcResult {
        QVector<double> coefficients;   ///< LPC系数(a1..ap)
        double reflectionFirst = 0.0;   ///< 首阶反射系数
        double predictionGain = 0.0;    ///< 预测增益(dB)
        double residualEnergy = 0.0;    ///< 残差能量
    };

    /** 分析统计 */
    struct Stats {
        quint64 totalAnalyses = 0;
        quint64 totalCoefficientsComputed = 0;
        double  averageProcessingTimeMs = 0.0;
    };

    explicit LinearPredictiveCoder(QObject* parent = nullptr);

    /** @brief LPC分析 @param data 信号 @param order 阶数 @return LPC结果 */
    LpcResult analyze(const QVector<double>& data, int order) const;

    /** @brief 合成(逆滤波) @param residual 残差 @param coefficients LPC系数 @return 合成信号 */
    QVector<double> synthesize(const QVector<double>& residual,
                               const QVector<double>& coefficients) const;

    /** @brief 计算反射系数 @param data 信号 @param maxOrder 最大阶数 @return 反射系数序列 */
    QVector<double> reflectionCoefficients(const QVector<double>& data,
                                           int maxOrder) const;

    /** @brief 计算LPC谱包络 @param coefficients LPC系数 @param numPoints 频率点数 @return (频率,幅度dB) */
    QVector<QPair<double, double>> spectralEnvelope(
        const QVector<double>& coefficients, int numPoints = 256) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void analysisCompleted(int order, double predictionGain);

private:
    /** 自相关 */
    static QVector<double> autocorrelation(const QVector<double>& data, int maxLag);
    /** Levinson-Durbin递推 */
    static bool levinsonDurbin(const QVector<double>& r, int order,
                               QVector<double>& a, double& error);

    mutable Stats m_stats;
    mutable double m_timeSum;
};

#endif // LINEARPREDICTIVECODER_H
