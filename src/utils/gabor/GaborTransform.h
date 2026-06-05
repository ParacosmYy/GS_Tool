/**
 * @file GaborTransform.h
 * @brief Gabor变换 — 时频分析(Gabor原子/加窗DFT)
 *
 * 功能: 使用Gabor核函数进行时频分析，支持可调窗口/频率采样，
 *       统计分析次数/帧数/耗时。
 */
#ifndef GABORTRANSFORM_H
#define GABORTRANSFORM_H

#include <QObject>
#include <QVector>

class GaborTransform : public QObject {
    Q_OBJECT
public:
    struct Stats {
        quint64 totalTransforms = 0;
        quint64 totalCoefficientsComputed = 0;
        double  averageProcessingTimeMs = 0.0;
    };

    explicit GaborTransform(QObject* parent = nullptr);

    /** @brief 设置参数 @param windowSize 窗口大小 @param sigma 高斯宽度 @param numFreqs 频率数 */
    void setParameters(int windowSize, double sigma, int numFreqs);

    /** @brief 执行Gabor变换 @param data 信号 @return 时频系数[freq][time] */
    QVector<QVector<double>> transform(const QVector<double>& data);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void transformCompleted(int frameCount, int numFreqs);

private:
    int m_windowSize;
    double m_sigma;
    int m_numFreqs;
    Stats m_stats;
    double m_timeSum;
};

#endif // GABORTRANSFORM_H
