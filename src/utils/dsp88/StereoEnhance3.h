#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 立体声增强处理器
 *
 * 扩展立体声声场宽度，增强空间感。
 */
class StereoEnhance3 : public QObject {
    Q_OBJECT
public:
    struct Stats {
        int totalSamplesProcessed = 0;
        int totalFramesApplied = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit StereoEnhance3(QObject* parent = nullptr);

    /** @brief 处理立体声帧(左/右声道) */
    QPair<QVector<double>, QVector<double>> process(const QVector<double>& left,
                                                     const QVector<double>& right);

    /** @brief 设置增强宽度(0~1) */
    void setWidth(double width);

    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    void enhanceApplied(int sampleCount, double width);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
    double m_width = 0.7;
};
