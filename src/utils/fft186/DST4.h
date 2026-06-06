/**
 * @file DST4.h
 * @brief 离散正弦变换IV型(反对称扩展+快速递推) — Discrete Sine Transform Type-IV via Antisymmetric Extension and Fast Recursion
 *
 * 功能: 实现DST-IV变换，支持反对称扩展方法、快速递推算法、
 *       正交归一化和批量变换处理。
 *
 * 协作: DCT4(DCT-IV) / DST1(DST-I) / MDCT6(MDCT)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief DST-IV处理器(反对称扩展+快速递推)
 */
class DST4 : public QObject {
    Q_OBJECT

public:
    /** @brief 运行统计 */
    struct Stats {
        quint64 totalTransforms = 0;
        int transformSize = 0;
        bool normalized = false;
        double avgProcessingTimeMs = 0.0;
    };

    explicit DST4(QObject *parent = nullptr);
    ~DST4() override;

    void setNormalized(bool enabled);

    /** @brief 正向DST-IV变换 */
    QVector<double> forward(const QVector<double>& input);

    /** @brief 逆向DST-IV变换(DST-IV是自逆的) */
    QVector<double> inverse(const QVector<double>& input);

    /** @brief 通过反对称扩展计算DST-IV */
    QVector<double> viaAntisymmetric(const QVector<double>& input) const;

    /** @brief 快速递推DST-IV */
    QVector<double> fastRecurse(const QVector<double>& input) const;

    /** @brief 生成DST-IV变换矩阵 */
    QVector<QVector<double>> transformMatrix(int N) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void transformCompleted(int size, double timeMs);

private:
    bool m_normalized = true;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Next power of 2 */
    int nextPow2(int n) const;

    /** @brief Radix-2 FFT */
    void fft(QVector<double>& re, QVector<double>& im, bool inv) const;
};
