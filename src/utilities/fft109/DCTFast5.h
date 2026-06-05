#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 快速DCT（离散余弦变换）实现 (版本5)
 *
 * 提供DCT-II和DCT-III的快速计算，基于FFT加速实现，适用于音频/图像压缩。
 */
class DCTFast5 : public QObject {
    Q_OBJECT
public:
    /// 统计信息结构
    struct Stats {
        int totalTransforms = 0;        ///< 总变换次数
        double avgProcessingTimeMs = 0.0; ///< 平均处理耗时(ms)
        int lastTransformSize = 0;      ///< 最近变换点数
    };

    explicit DCTFast5(QObject* parent = nullptr);
    Stats stats() const { return m_stats; }
    void resetStatistics();

    /**
     * @brief 执行DCT-II（前向变换）
     * @param samples 输入时域采样
     * @return DCT系数序列
     */
    QVector<double> forward(const QVector<double>& samples);

    /**
     * @brief 执行DCT-III（逆变换）
     * @param coefficients DCT系数序列
     * @return 重建的时域采样
     */
    QVector<double> inverse(const QVector<double>& coefficients);

    /**
     * @brief 保留前N个系数重建信号
     * @param samples 原始采样
     * @param keepCoefficients 保留的系数数量
     * @return 重建后的采样
     */
    QVector<double> reconstruct(const QVector<double>& samples, int keepCoefficients);

    /**
     * @brief 计算DCT压缩的能量保留比
     * @param samples 原始采样
     * @param keepCoefficients 保留的系数数量
     * @return 能量保留百分比 [0.0, 1.0]
     */
    double energyRetention(const QVector<double>& samples, int keepCoefficients) const;

signals:
    /// 变换完成信号
    void transformCompleted(int pointCount);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
};
