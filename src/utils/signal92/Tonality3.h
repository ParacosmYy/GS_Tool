#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 音调分析工具类
 *
 * 计算信号的音调属性指标，用于评估信号的
 * 音调强度与纯度。
 */
class Tonality3 : public QObject {
    Q_OBJECT
public:
    /// 计算统计信息
    struct Stats {
        int totalComputations = 0;  ///< 总计算次数
        double avgProcessingTimeMs = 0.0; ///< 平均处理耗时(ms)
    };

    explicit Tonality3(QObject* parent = nullptr);

    /** @brief 设置分析帧大小(采样点数) */
    void setFrameSize(int size);

    /** @brief 对输入信号帧计算音调指标 */
    double compute(const QVector<double>& frame);

    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 计算完成信号，返回音调值 */
    void computed(double tonality);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
    int m_frameSize = 1024;
};
