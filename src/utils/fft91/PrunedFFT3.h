#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 剪枝FFT工具类
 *
 * 提供剪枝快速傅里叶变换功能，通过设置剪枝掩码
 * 仅计算感兴趣的频率分量，减少计算量。
 */
class PrunedFFT3 : public QObject {
    Q_OBJECT
public:
    /// 计算统计信息
    struct Stats {
        int totalComputations = 0;  ///< 总计算次数
        double avgProcessingTimeMs = 0.0; ///< 平均处理耗时(ms)
    };

    explicit PrunedFFT3(QObject* parent = nullptr);

    /** @brief 设置剪枝掩码(true=计算该频率分量) */
    void setPruneMask(const QVector<bool>& mask);

    /** @brief 对输入信号执行剪枝FFT计算 */
    QVector<double> compute(const QVector<double>& input);

    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 计算完成信号，返回输出频谱点数 */
    void computationCompleted(int spectrumSize);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
    QVector<bool> m_pruneMask;
};
