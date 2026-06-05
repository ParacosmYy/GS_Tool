/**
 * @file SpectralEstimator.h
 * @brief 谱估计器 — AR/MA/ARMA参数化方法
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 参数化谱估计器
 * AR(Yule-Walker/Burg), MA, ARMA模型功率谱估计
 */
class SpectralEstimator : public QObject
{
    Q_OBJECT

public:
    /** @brief 模型类型 */
    enum ModelType {
        AR,        ///< 自回归模型
        MA,        ///< 滑动平均模型
        ARMA       ///< 自回归滑动平均模型
    };

    /** @brief 统计信息 */
    struct Stats {
        int totalEstimates = 0;
        int totalSamplesProcessed = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit SpectralEstimator(QObject* parent = nullptr);

    /** @brief AR模型估计(Yule-Walker) @param signal 输入信号 @param order AR阶数 */
    void estimateAR(const QVector<double>& signal, int order);

    /** @brief AR模型估计(Burg) @param signal 输入信号 @param order AR阶数 */
    void estimateARBurg(const QVector<double>& signal, int order);

    /** @brief ARMA模型估计 @param signal 输入信号 @param arOrder AR阶数 @param maOrder MA阶数 */
    void estimateARMA(const QVector<double>& signal, int arOrder, int maOrder);

    /** @brief 计算功率谱密度 @param freqs 频率点 @param sampleRate 采样率 @return PSD值(dB/Hz) */
    QVector<double> powerSpectralDensity(const QVector<double>& freqs,
                                          double sampleRate) const;

    /** @brief 获取AR系数 */
    QVector<double> arCoefficients() const { return m_arCoeffs; }
    /** @brief 获取MA系数 */
    QVector<double> maCoefficients() const { return m_maCoeffs; }
    /** @brief 获取噪声方差 */
    double noiseVariance() const { return m_noiseVar; }
    /** @brief 获取模型类型 */
    ModelType modelType() const { return m_modelType; }
    /** @brief 获取AIC */
    double aic() const { return m_aic; }
    /** @brief 获取模型阶数 */
    int order() const;

    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    void estimateCompleted(ModelType type, int order);

private:
    /** @brief Levinson-Durbin递归 */
    QVector<double> levinsonDurbin(const QVector<double>& autocorr, int order,
                                   double& noiseVar) const;

    /** @brief 计算自相关 */
    QVector<double> autocorrelation(const QVector<double>& signal, int maxLag) const;

    ModelType m_modelType = AR;
    QVector<double> m_arCoeffs;
    QVector<double> m_maCoeffs;
    double m_noiseVar = 1.0;
    double m_aic = 0.0;

    Stats m_stats;
    double m_timeSum = 0.0;
};
