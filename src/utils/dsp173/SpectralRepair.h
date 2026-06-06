/**
 * @file SpectralRepair.h
 * @brief 频谱修复(损坏频率bin插值+伪影抑制) — Spectral Repair with Interpolation of Damaged Frequency Bins and Artifact Reduction
 *
 * 功能: 实现频谱修复算法，支持损坏频率bin检测、邻域插值修复、
 *       伪影抑制和时频掩蔽。
 *
 * 协作: SplitRadixFFT(FFT) / SpectralSubtraction3(谱减) / WienerFilter5(维纳滤波)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 频谱修复器
 */
class SpectralRepair : public QObject {
    Q_OBJECT

public:
    /** @brief 运行统计 */
    struct Stats {
        quint64 totalRepairs = 0;          ///< 累计修复次数
        int lastRepairedBins = 0;          ///< 最近修复的bin数
        double avgProcessingTimeMs = 0.0;  ///< 平均耗时(ms)
    };

    explicit SpectralRepair(QObject *parent = nullptr);
    ~SpectralRepair() override;

    void setDamageThreshold(double threshold);
    void setInterpolationWidth(int width);
    void setArtifactSuppression(double factor);

    /**
     * @brief 检测并修复损坏的频率bin
     * @param magnitude 幅度谱
     * @param phase 相位谱
     * @return 修复后的幅度谱
     */
    QVector<double> repair(const QVector<double>& magnitude,
                           const QVector<double>& phase);

    /**
     * @brief 标记损坏的bin索引
     * @param magnitude 幅度谱
     * @return 损坏的bin索引列表
     */
    QVector<int> detectDamagedBins(const QVector<double>& magnitude) const;

    /** @brief 邻域插值修复单个bin */
    double interpolateBin(const QVector<double>& magnitude, int bin, int width) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void repairCompleted(int repairedBins);

private:
    /** @brief 计算局部统计量(均值/标准差) */
    void localStats(const QVector<double>& mag, int bin, int win,
                    double& mean, double& stddev) const;

    /** @brief 应用伪影抑制 */
    void suppressArtifacts(QVector<double>& magnitude,
                           const QVector<int>& repairedBins) const;

    double m_threshold = 3.0;   ///< 损坏检测阈值(标准差倍数)
    int m_width = 5;            ///< 插值邻域宽度
    double m_suppression = 0.5; ///< 伪影抑制因子

    Stats m_stats;
    double m_timeSum = 0.0;
};
