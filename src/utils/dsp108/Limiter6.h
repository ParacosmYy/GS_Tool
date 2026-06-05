#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 信号限幅器实现 (版本6)
 *
 * 提供砖墙限制器功能，确保信号不超过指定阈值，支持lookahead和自动增益。
 */
class Limiter6 : public QObject {
    Q_OBJECT
public:
    /// 统计信息结构
    struct Stats {
        int totalLimitingRuns = 0;      ///< 总限幅处理次数
        double avgProcessingTimeMs = 0.0; ///< 平均处理耗时(ms)
        double peakOutputDb = -120.0;   ///< 峰值输出(dB)
    };

    explicit Limiter6(QObject* parent = nullptr);
    Stats stats() const { return m_stats; }
    void resetStatistics();

    /**
     * @brief 处理采样数据限幅
     * @param samples 输入采样数据
     * @return 限幅后的采样数据
     */
    QVector<double> process(const QVector<double>& samples);

    /**
     * @brief 设置限幅参数
     * @param ceilingDb 限幅上限(dB)
     * @param releaseMs 释放时间(ms)
     */
    void setParameters(double ceilingDb, double releaseMs);

    /**
     * @brief 设置lookahead时间
     * @param lookaheadMs lookahead延迟(ms)
     */
    void setLookahead(double lookaheadMs);

    /**
     * @brief 获取当前增益衰减
     * @return 增益衰减量(dB)
     */
    double gainReductionDb() const { return m_gainReductionDb; }

signals:
    /// 限幅处理完成信号
    void limitingCompleted(int sampleCount);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
    double m_ceilingDb = -0.3;
    double m_releaseMs = 50.0;
    double m_lookaheadMs = 5.0;
    double m_gainReductionDb = 0.0;
};
