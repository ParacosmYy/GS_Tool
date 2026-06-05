#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 噪声门处理器实现 (版本5)
 *
 * 提供信号噪声门功能，当信号低于阈值时静音或衰减，用于去除背景噪声。
 */
class Gate5 : public QObject {
    Q_OBJECT
public:
    /// 统计信息结构
    struct Stats {
        int totalGateRuns = 0;          ///< 总门控处理次数
        double avgProcessingTimeMs = 0.0; ///< 平均处理耗时(ms)
        double gateOpenRatio = 0.0;     ///< 门开启时间比例
    };

    explicit Gate5(QObject* parent = nullptr);
    Stats stats() const { return m_stats; }
    void resetStatistics();

    /**
     * @brief 处理采样数据
     * @param samples 输入采样数据
     * @return 门控处理后的采样数据
     */
    QVector<double> process(const QVector<double>& samples);

    /**
     * @brief 设置门控参数
     * @param thresholdDb 门控阈值(dB)
     * @param rangeDb 门控范围/衰减量(dB)
     * @param attackMs 启动时间(ms)
     * @param holdMs 保持时间(ms)
     * @param releaseMs 释放时间(ms)
     */
    void setParameters(double thresholdDb, double rangeDb, double attackMs, double holdMs, double releaseMs);

    /**
     * @brief 获取当前门状态
     * @return 是否开启（信号通过）
     */
    bool isOpen() const { return m_isOpen; }

signals:
    /// 门控处理完成信号
    void gateCompleted(int sampleCount);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
    double m_thresholdDb = -40.0;
    double m_rangeDb = -80.0;
    double m_attackMs = 1.0;
    double m_holdMs = 50.0;
    double m_releaseMs = 100.0;
    bool m_isOpen = false;
    double m_envelope = 0.0;
    double m_holdTimer = 0.0;
};
