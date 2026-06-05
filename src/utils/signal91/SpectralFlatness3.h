#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 频谱平坦度分析工具类
 *
 * 计算信号的频谱平坦度指标(几何平均与算术平均之比)，
 * 用于区分类噪声信号与类音调信号。
 */
class SpectralFlatness3 : public QObject {
    Q_OBJECT
public:
    /// 计算统计信息
    struct Stats {
        int totalComputations = 0;  ///< 总计算次数
        double avgProcessingTimeMs = 0.0; ///< 平均处理耗时(ms)
    };

    explicit SpectralFlatness3(QObject* parent = nullptr);

    /** @brief 设置分析帧大小(采样点数) */
    void setFrameSize(int size);

    /** @brief 对输入信号帧计算频谱平坦度 */
    double compute(const QVector<double>& frame);

    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 计算完成信号，返回频谱平坦度值 */
    void computed(double flatness);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
    int m_frameSize = 1024;
};
