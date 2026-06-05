/**
 * @file SimulatedAnneal2.h
 * @brief string algorithm module - SimulatedAnneal2
 */
#pragma once
#include <QObject>
#include <QVector>
class SimulatedAnneal2 : public QObject {
    Q_OBJECT
public:
    struct Stats { quint64 calls = 0; quint64 items = 0; quint64 errors = 0; };
    explicit SimulatedAnneal2(QObject *p = nullptr) : QObject(p) {}
    ~SimulatedAnneal2() override = default;
    QVector<double> compute(const QVector<double> &input);
    Stats stats() const { return m_stats; }
    void resetStats() { m_stats = {}; }
signals:
    void computed(const QVector<double> &result);
private:
    Stats m_stats;
};

