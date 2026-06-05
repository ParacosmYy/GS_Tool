/**
 * @file HilbertTransform.h
 * @brief Hilbert变换 — 解析信号/瞬时频率/包络
 *
 * 功能: 频域Hilbert变换，解析信号构造，瞬时频率估计，
 *       统计变换次数/耗时，变换完成信号。
 */
#ifndef HILBERTTRANSFORM_H
#define HILBERTTRANSFORM_H

#include <QObject>
#include <QVector>
#include <complex>

class HilbertTransformer : public QObject {
    Q_OBJECT
public:
    /** 操作统计 */
    struct Stats {
        quint64 totalTransforms = 0;
        double  avgProcessingTimeMs = 0.0;
    };

    explicit HilbertTransformer(QObject* parent = nullptr);

    /** @brief Hilbert变换(频域) @param signal 输入信号 @return 变换结果 */
    QVector<double> transform(const QVector<double>& signal);

    /** @brief 解析信号 @param signal 输入信号 @return 复数解析信号 */
    QVector<std::complex<double>> analyticSignal(
        const QVector<double>& signal);

    /** @brief 瞬时频率 @param signal 输入信号 @param sampleRate 采样率 @return 瞬时频率 */
    QVector<double> instantaneousFrequency(
        const QVector<double>& signal, double sampleRate);

    /** @brief 包络 @param signal 输入信号 @return 包络(幅度) */
    QVector<double> envelope(const QVector<double>& signal);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 变换完成 @param length 信号长度 */
    void transformCompleted(int length);

private:
    void fft(QVector<std::complex<double>>& data, bool inverse);
    int nextPow2(int n) const;

    Stats m_stats;
    double m_timeSum;
};

#endif // HILBERTTRANSFORM_H
