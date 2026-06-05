#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 频谱平坦度分析实现 (版本5)
 *
 * 计算信号的频谱平坦度（维纳熵），用于判断信号是类噪声还是类音调。
 */
class SpectralFlatness5 : public QObject {
    Q_OBJECT
public:
    /// 统计信息结构
    struct Stats {
        int totalAnalyses = 0;          ///< 总分析次数
        double avgProcessingTimeMs = 0.0; ///< 平均处理耗时(ms)
        double lastFlatness = 0.0;      ///< 最近一次平坦度值
    };

    explicit SpectralFlatness5(QObject* parent = nullptr);
    Stats stats() const { return m_stats; }
    void resetStatistics();

    /**
     * @brief 从功率谱计算频谱平坦度
     * @param powerSpectrum 功率谱密度
     * @return 频谱平坦度 [0.0, 1.0]，1.0表示完全平坦
     */
    double compute(const QVector<double>& powerSpectrum);

    /**
     * @brief 从时域采样直接计算频谱平坦度
     * @param samples 时域采样数据
     * @return 频谱平坦度
     */
    double computeFromTimeDomain(const QVector<double>& samples);

    /**
     * @brief 计算指定频段的平坦度
     * @param powerSpectrum 功率谱密度
     * @param startBin 起始频率bin索引
     * @param endBin 结束频率bin索引
     * @return 子带频谱平坦度
     */
    double computeBand(const QVector<double>& powerSpectrum, int startBin, int endBin);

    /**
     * @brief 判断信号是否为类噪声
     * @param threshold 判定阈值，默认0.8
     * @return 是否为类噪声信号
     */
    bool isNoiseLike(double threshold = 0.8) const;

signals:
    /// 分析完成信号
    void analysisCompleted(double flatness);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
    double m_lastFlatness = 0.0;
};
