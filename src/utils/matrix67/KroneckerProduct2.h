#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

class KroneckerProduct2 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalProducts = 0; int totalDimensions = 0; double avgProcessingTimeMs = 0.0; };
    explicit KroneckerProduct2(QObject* parent = nullptr);
    void setMatrixA(const QVector<QVector<double>>& A);
    void setMatrixB(const QVector<QVector<double>>& B);
    QVector<QVector<double>> compute();
    QVector<QVector<double>> result() const { return m_result; }
    int rows() const { return m_rows; }
    int cols() const { return m_cols; }
    Stats stats() const { return m_stats; }
    void resetStatistics();
signals:
    void computed(int rows, int cols);
private:
    int m_rows = 0; int m_cols = 0;
    QVector<QVector<double>> m_A; QVector<QVector<double>> m_B;
    QVector<QVector<double>> m_result;
    Stats m_stats; double m_timeSum = 0.0;
};
