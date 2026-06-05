#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 递归离散傅里叶变换
 *
 * 基于Cooley-Tukey递归实现DFT/IDFT，支持任意长度输入，
 * 适用于中等规模频谱分析场景。
 */
class RecursiveDFT4 : public QObject {
    Q_OBJECT
public:
    /// 统计信息结构体
    struct Stats {
        int totalComputations = 0;   ///< 已完成计算次数
        double avgProcessingTimeMs = 0.0; ///< 平均处理耗时(ms)
    };

    explicit RecursiveDFT4(QObject* parent = nullptr);

    /** @brief 设置变换点数 */
    void setSize(int n);
    /** @brief 正向DFT变换 */
    QVector<QPair<double, double>> compute(const QVector<double>& samples);
    /** @brief 逆向DFT变换 */
    QVector<double> inverse(const QVector<QPair<double, double>>& spectrum);

    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 计算完成，返回输出点数 */
    void computationCompleted(int pointCount);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
    int m_size = 256;
};
