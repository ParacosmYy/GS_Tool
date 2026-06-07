/**
 * @file WinogradFFT5.h
 * @brief Winograd大FFT(嵌套小Winograd变换+模算术) — Winograd Large FFT via Nesting Small Winograd Transforms with Modular Arithmetic
 *
 * 功能: 实现Winograd大FFT算法，支持小Winograd变换嵌套、
 *       模算术优化和最小乘法数DFT计算。
 *
 * 协作: PrimeFactorFFT5(素因子FFT) / ChirpZ5(线性调频Z变换) / FftEngine3(FFT引擎)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief Winograd大FFT(嵌套小变换+模算术)
 */
class WinogradFFT5 : public QObject {
    Q_OBJECT

public:
    /** @brief 运行统计 */
    struct Stats {
        quint64 totalTransforms = 0;
        int lastSize = 0;
        int numMultiplies = 0;
        int numAdditions = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit WinogradFFT5(QObject *parent = nullptr);
    ~WinogradFFT5() override;

    /** @brief Compute forward FFT using Winograd algorithm */
    QVector<double> forward(const QVector<double>& input);

    /** @brief Compute inverse FFT */
    QVector<double> inverse(const QVector<double>& spectrum);

    /** @brief Get supported Winograd transform sizes */
    QVector<int> supportedSizes() const;

    /** @brief Count multiplications for size n */
    int countMultiplies(int n) const;

    /** @brief Count additions for size n */
    int countAdditions(int n) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void transformCompleted(int n, int muls, int adds, double timeMs);

private:
    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Winograd-2 inner transform: 2 mul, 4 add */
    QVector<QVector<double>> winograd2(
        const QVector<QVector<double>>& x, bool inv) const;

    /** @brief Winograd-3 inner transform: 3 mul, 9 add */
    QVector<QVector<double>> winograd3(
        const QVector<QVector<double>>& x, bool inv) const;

    /** @brief Winograd-4 inner transform: 5 mul, 13 add */
    QVector<QVector<double>> winograd4(
        const QVector<QVector<double>>& x, bool inv) const;

    /** @brief Winograd-5 inner transform: 5 mul, 17 add */
    QVector<QVector<double>> winograd5(
        const QVector<QVector<double>>& x, bool inv) const;

    /** @brief Winograd-7 inner transform: 8 mul, 26 add */
    QVector<QVector<double>> winograd7(
        const QVector<QVector<double>>& x, bool inv) const;

    /** @brief Nest two Winograd transforms */
    QVector<QVector<double>> nestTransforms(
        const QVector<QVector<double>>& x, int n1, int n2, bool inv) const;

    /** @brief Factorize into supported Winograd sizes */
    QVector<int> winogradFactorize(int n) const;

    /** @brief Flat <-> complex conversion */
    static QVector<double> toFlat(const QVector<QVector<double>>& c);
    static QVector<QVector<double>> fromFlat(const QVector<double>& f);
};
