#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 递归DFT实现 (版本5)
 *
 * 提供基于递归分治策略的DFT计算，适用于教育和小规模数据的频谱分析。
 */
class RecursiveDFT5 : public QObject {
    Q_OBJECT
public:
    /// 统计信息结构
    struct Stats {
        int totalTransforms = 0;        ///< 总变换次数
        double avgProcessingTimeMs = 0.0; ///< 平均处理耗时(ms)
        int maxRecursionDepth = 0;      ///< 最大递归深度
    };

    explicit RecursiveDFT5(QObject* parent = nullptr);
    Stats stats() const { return m_stats; }
    void resetStatistics();

    /**
     * @brief 执行前向DFT变换
     * @param samples 输入时域采样
     * @return 复数频谱 [实部, 虚部] 对
     */
    QVector<QPair<double, double>> forward(const QVector<double>& samples);

    /**
     * @brief 执行逆DFT变换
     * @param spectrum 复数频谱 [实部, 虚部] 对
     * @return 重建的时域采样
     */
    QVector<double> inverse(const QVector<QPair<double, double>>& spectrum);

    /**
     * @brief 计算指定频率点的DFT
     * @param samples 时域采样
     * @param freqIndex 频率bin索引
     * @return 该频率点的复数值
     */
    QPair<double, double> singleBin(const QVector<double>& samples, int freqIndex) const;

    /**
     * @brief 设置递归深度限制
     * @param maxDepth 最大递归深度，0表示自动
     */
    void setMaxRecursionDepth(int maxDepth) { m_maxDepth = maxDepth; }

signals:
    /// 变换完成信号
    void transformCompleted(int pointCount);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
    int m_maxDepth = 0;
};
