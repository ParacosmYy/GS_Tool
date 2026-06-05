#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 剪枝FFT实现 (版本5)
 *
 * 仅计算指定频率bin的FFT输出，跳过不需要的蝶形运算，适用于大点数FFT的频段分析。
 */
class PrunedFFT5 : public QObject {
    Q_OBJECT
public:
    /// 统计信息结构
    struct Stats {
        int totalTransforms = 0;        ///< 总变换次数
        double avgProcessingTimeMs = 0.0; ///< 平均处理耗时(ms)
        double savingsRatio = 0.0;      ///< 计算节省比例
    };

    explicit PrunedFFT5(QObject* parent = nullptr);
    Stats stats() const { return m_stats; }
    void resetStatistics();

    /**
     * @brief 执行剪枝FFT变换
     * @param samples 输入时域采样
     * @param outputBins 需要输出的频率bin索引列表
     * @return 指定bin的复数频谱 [实部, 虚部] 对
     */
    QVector<QPair<double, double>> transform(const QVector<double>& samples, const QVector<int>& outputBins);

    /**
     * @brief 设置FFT点数（自动补零到2的幂）
     * @param fftSize 目标FFT点数
     */
    void setFFTSize(int fftSize);

    /**
     * @brief 设置频率范围模式（替代逐bin指定）
     * @param startBin 起始bin索引
     * @param endBin 结束bin索引
     */
    void setFrequencyRange(int startBin, int endBin);

    /**
     * @brief 获取计算节省率
     * @return 节省的计算量百分比 [0.0, 1.0]
     */
    double computeSavings() const { return m_savingsRatio; }

signals:
    /// 变换完成信号
    void transformCompleted(int outputBinCount);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
    int m_fftSize = 1024;
    double m_savingsRatio = 0.0;
    QVector<int> m_outputBins;
};
