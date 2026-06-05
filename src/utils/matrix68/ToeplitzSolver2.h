#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

class ToeplitzSolver2 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalSolves = 0; int totalDimensions = 0; double avgProcessingTimeMs = 0.0; };
    explicit ToeplitzSolver2(QObject* parent = nullptr);
    void setColumn(const QVector<double>& col);
    void setRow(const QVector<double>& row);
    QVector<double> solve(const QVector<double>& rhs);
    double determinant() const { return m_det; }
    bool isHermitian() const { return m_hermitian; }
    Stats stats() const { return m_stats; }
    void resetStatistics();
signals:
    void solveCompleted(int n, double residual);
private:
    int m_n = 0; double m_det = 1.0; bool m_hermitian = false;
    QVector<double> m_col; QVector<double> m_row;
    QVector<double> levinsonDurbin(const QVector<double>& t, const QVector<double>& b);
    Stats m_stats; double m_timeSum = 0.0;
};
