/**
 * @file SplineInterp4.h
 * @brief dsp algorithm module - SplineInterp4
 */
#pragma once
#include <QObject>
#include <QVector>
class SplineInterp4 : public QObject {
    Q_OBJECT
public:
    struct Stats { quint64 calls = 0; quint64 items = 0; quint64 errors = 0; };
    explicit SplineInterp4(QObject *p = nullptr) : QObject(p) {}
    ~SplineInterp4() override = default;
    QVector<double> compute(const QVector<double> &input);
    Stats stats() const { return m_stats; }
    void resetStats() { m_stats = {}; }
signals:
    void computed(const QVector<double> &result);
private:
    Stats m_stats;
};

