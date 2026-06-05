/**
 * @file FenwickTree3.h
 * @brief Fenwick树3 — 二维范围+区间加+点查
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

class FenwickTree3 : public QObject
{
    Q_OBJECT

public:
    struct Stats {
        int totalUpdates = 0;
        int totalQueries = 0;
        int totalRangeQueries = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit FenwickTree3(int rows = 0, int cols = 0, QObject* parent = nullptr);

    void resize(int rows, int cols);
    void add(int row, int col, double delta);
    void addRange(int r1, int c1, int r2, int c2, double delta);
    double sum(int row, int col) const;
    double rangeSum(int r1, int c1, int r2, int c2) const;
    double get(int row, int col) const;
    void set(int row, int col, double value);
    void clear();

    int rows() const { return m_rows; }
    int cols() const { return m_cols; }
    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    void updated(int row, int col, double value);

private:
    int m_rows = 0;
    int m_cols = 0;
    QVector<QVector<double>> m_tree1;
    QVector<QVector<double>> m_tree2;
    QVector<QVector<double>> m_tree3;
    QVector<QVector<double>> m_tree4;

    void internalAdd(QVector<QVector<double>>& tree, int r, int c, double delta);
    double internalSum(const QVector<QVector<double>>& tree, int r, int c) const;

    Stats m_stats;
    double m_timeSum = 0.0;
};
