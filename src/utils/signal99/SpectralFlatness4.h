#pragma once
#include <QObject>
#include <QVector>

/**
 * @brief 频谱平坦度计算器
 *
 * 计算信号频谱的几何均值与算术均值之比，
 * 用于区分类噪声音(平坦度高)与音调信号(平坦度低)。
 */
class SpectralFlatness4 : public QObject {
    Q_OBJECT
public:
    /// 统计信息结构体
    struct Stats {
        int totalFrames = 0;         ///< 已处理帧数
        double avgFlatness = 0.0;    ///< 平均平坦度
        double avgProcessingTimeMs = 0.0; ///< 平均处理耗时(ms)
    };

    explicit SpectralFlatness4(QObject* parent = nullptr);

    /** @brief 设置帧大小(采样点数) */
    void setFrameSize(int size);
    /** @brief 设置帧移步长 */
    void setHopSize(int hop);
    /** @brief 计算频谱平坦度 */
    double compute(const QVector<double>& spectrum);

    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 计算完成，返回平坦度值 */
    void computed(double flatness);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
    int m_frameSize = 1024;
    int m_hopSize = 512;
};
