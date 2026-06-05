/**
 * @file ToeplitzSolver2.h
 * @brief Toeplitz solver enhanced - Levinson/Trench/Yule-Walker
 */
#pragma once
#include <QObject>
#include <QVector>
class ToeplitzSolver2 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalSolves = 0; int totalSystemsSize = 0; double avgProcessingTimeMs = 0.0; };
    explicit ToeplitzSolver2(QObject* parent = nullptr);
    QVector<double> solve(const QVector<double>& col, const QVector<double>& rhs);
    QVector<double> inverse(const QVector<double>& col);
    double determinant(const QVector<double>& col);
    QVector<double> yuleWalker(const QVector<double>& autocorr, int order);
    Stats stats() const { return m_stats; }
    void resetStatistics();
signals:
    void solveComplete(int n);
private:
    int m_n = 0;
    Stats m_stats; double m_timeSum = 0.0;
};
