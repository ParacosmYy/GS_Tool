/**
 * @file SpectralRepair2.h
 * @brief 频谱修复(自回归模型缺失数据插值+间隙填充) — Spectral Repair with Missing Data Interpolation via Autoregressive Model and Gap Filling
 *
 * 功能: 实现频谱修复算法，支持自回归(AR)模型参数估计、
 *       缺失数据前向后向插值、频谱间隙填充和连续性恢复。
 *
 * 协作: LinearPredictiveCoding4(LPC分析) / WienerFilter4(维纳滤波) / SignalDenoiser3(信号去噪)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 频谱修复器(AR模型插值+间隙填充)
 */
class SpectralRepair2 : public QObject {
    Q_OBJECT

public:
    /** @brief 运行统计 */
    struct Stats {
        quint64 totalRepairs = 0;
        int signalLength = 0;
        int gapCount = 0;
        int arOrder = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit SpectralRepair2(QObject *parent = nullptr);
    ~SpectralRepair2() override;

    void setArOrder(int order);
    void setMaxGapLength(int len);

    /** @brief 检测缺失数据位置(NaN标记) */
    QVector<QPair<int, int>> detectGaps(const QVector<double>& signal) const;

    /** @brief 修复信号中的缺失数据 */
    QVector<double> repair(const QVector<double>& signal);

    /** @brief AR模型参数估计(Yule-Walker) */
    QVector<double> estimateArParams(const QVector<double>& segment) const;

    /** @brief 前向预测 */
    double forwardPredict(const QVector<double>& signal, int pos,
                          const QVector<double>& ar) const;

    /** @brief 后向预测 */
    double backwardPredict(const QVector<double>& signal, int pos,
                           const QVector<double>& ar) const;

    /** @brief 用已知数据估计AR参数 */
    QVector<double> trainArModel(const QVector<double>& signal) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void repairCompleted(int gapsFilled, double timeMs);

private:
    int m_arOrder = 16;
    int m_maxGapLength = 256;
    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Levinson-Durbin recursion for AR parameters */
    QVector<double> levinsonDurbin(const QVector<double>& autocorr) const;

    /** @brief Autocorrelation estimation */
    QVector<double> autocorrelation(const QVector<double>& seg, int maxLag) const;

    /** @brief Blend forward and backward predictions across gap */
    void fillGap(QVector<double>& signal, int start, int end,
                 const QVector<double>& ar) const;
};
