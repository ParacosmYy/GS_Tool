#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief StereoWidth4 - 立体声宽度控制器
 *
 * 通过中间/侧边(M/S)处理控制立体声宽度，
 * 支持宽度调节、单声化平衡和相位一致性检查。
 */
class StereoWidth4 : public QObject {
    Q_OBJECT
public:
    struct Stats {
        int totalFramesProcessed = 0;
        int totalPhaseWarnings = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit StereoWidth4(QObject* parent = nullptr);

    /** @brief 设置宽度因子(0=单声, 1=原始, 2=扩展) */
    void setWidth(double width);

    /** @brief 处理立体声帧(左右声道) */
    QPair<QVector<double>, QVector<double>> process(
        const QVector<double>& left, const QVector<double>& right);

    /** @brief 计算当前帧的立体声相关系数 */
    double correlation(const QVector<double>& left, const QVector<double>& right) const;

    /** @brief 获取M/S编码后的中间和侧边信号 */
    QPair<QVector<double>, QVector<double>> toMS(
        const QVector<double>& left, const QVector<double>& right) const;

    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    void phaseWarningDetected(int sampleIndex);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
    double m_width = 1.0;
};
