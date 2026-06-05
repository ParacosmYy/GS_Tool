#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 动态压缩器工具类
 *
 * 提供音频动态范围压缩功能，支持设置阈值、压缩比、
 * 启动时间等参数，用于信号电平控制。
 */
class Compressor4 : public QObject {
    Q_OBJECT
public:
    /// 压缩处理统计信息
    struct Stats {
        int totalProcessed = 0;     ///< 总处理帧数
        double avgProcessingTimeMs = 0.0; ///< 平均处理耗时(ms)
    };

    explicit Compressor4(QObject* parent = nullptr);

    /** @brief 设置压缩阈值(dB) */
    void setThreshold(double thresholdDb);

    /** @brief 设置压缩比 */
    void setRatio(double ratio);

    /** @brief 设置启动时间(ms) */
    void setAttack(double attackMs);

    /** @brief 对输入信号帧执行动态压缩 */
    QVector<double> process(const QVector<double>& input);

    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 处理完成信号，返回输出帧数 */
    void processingCompleted(int frameCount);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
    double m_threshold = -20.0;
    double m_ratio = 4.0;
    double m_attack = 10.0;
};
