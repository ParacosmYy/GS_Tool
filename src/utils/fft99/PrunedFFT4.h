#pragma once
#include <QObject>
#include <QVector>

/**
 * @brief 剪枝FFT变换器
 *
 * 当只需要部分频率分量的FFT结果时，通过剪枝蝶形网络
 * 减少计算量，适用于频谱监控等稀疏频谱场景。
 */
class PrunedFFT4 : public QObject {
    Q_OBJECT
public:
    /// 统计信息结构体
    struct Stats {
        int totalComputations = 0;   ///< 已完成计算次数
        int prunedBins = 0;          ///< 已剪枝频点数
        double avgProcessingTimeMs = 0.0; ///< 平均处理耗时(ms)
    };

    explicit PrunedFFT4(QObject* parent = nullptr);

    /** @brief 设置FFT点数(2的幂) */
    void setSize(int n);
    /** @brief 设置剪枝掩码，true表示保留该频点 */
    void setPruneMask(const QVector<bool>& mask);
    /** @brief 执行剪枝FFT计算 */
    QVector<double> compute(const QVector<double>& samples);

    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 计算完成，返回输出频点数 */
    void computationCompleted(int outputBins);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
    int m_size = 256;
    QVector<bool> m_pruneMask;
};
