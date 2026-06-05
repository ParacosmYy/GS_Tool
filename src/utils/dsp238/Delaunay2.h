/**
 * @file Delaunay2.h
 * @brief dsp algorithm module - Delaunay2
 */
#pragma once
#include <QObject>
#include <QVector>
class Delaunay2 : public QObject {
    Q_OBJECT
public:
    struct Stats { quint64 calls = 0; quint64 items = 0; quint64 errors = 0; };
    explicit Delaunay2(QObject *p = nullptr) : QObject(p) {}
    ~Delaunay2() override = default;
    QVector<double> compute(const QVector<double> &input);
    Stats stats() const { return m_stats; }
    void resetStats() { m_stats = {}; }
signals:
    void computed(const QVector<double> &result);
private:
    Stats m_stats;
};

