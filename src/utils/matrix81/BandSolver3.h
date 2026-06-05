#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief BandSolver3 - 带状线性系统求解器
 *
 * 针对带状矩阵优化的直接求解器，利用带状结构
 * 进行LU分解，复杂度为O(n*bw^2)而非O(n^3)。
 */
class BandSolver3 : public QObject {
    Q_OBJECT
public:
    struct Stats {
        int totalFactorizations = 0;
        int totalSolves = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit BandSolver3(QObject* parent = nullptr);

    /** @brief 设置半带宽并分解带状矩阵 */
    bool factorize(const QVector<QVector<double>>& bands, int n, int halfBandwidth);

    /** @brief 求解已分解的系统 */
    QVector<double> solve(const QVector<double>& rhs) const;

    /** @brief 获取半带宽 */
    int halfBandwidth() const;

    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    void factorizationCompleted(int bandwidth, int size);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
    int m_n = 0;
    int m_halfBandwidth = 0;
    QVector<QVector<double>> m_factoredBands;
};
