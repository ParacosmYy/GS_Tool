#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 递归DFT工具类
 *
 * 提供递归实现的离散傅里叶变换功能，支持设置变换长度，
 * 适用于教学和小规模频谱分析场景。
 */
class RecursiveDFT3 : public QObject {
    Q_OBJECT
public:
    /// 计算统计信息
    struct Stats {
        int totalComputations = 0;  ///< 总计算次数
        double avgProcessingTimeMs = 0.0; ///< 平均处理耗时(ms)
    };

    explicit RecursiveDFT3(QObject* parent = nullptr);

    /** @brief 设置DFT变换长度 */
    void setSize(int size);

    /** @brief 对输入信号执行递归DFT计算 */
    QVector<double> compute(const QVector<double>& input);

    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 计算完成信号，返回输出频谱点数 */
    void computationCompleted(int spectrumSize);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
    int m_size = 256;
};
