/**
 * @file matrix__605.h
 * @brief matrix module matrix__605
 */
#pragma once
#include <QObject>
#include <QVector>
class matrix__605 : public QObject {
    Q_OBJECT
public:
    struct Stats { quint64 calls=0; quint64 items=0; quint64 errors=0; };
    explicit matrix__605(QObject *p=nullptr) : QObject(p) {}
    ~matrix__605() override = default;
    QVector<double> compute(const QVector<double> &input);
    Stats stats() const { return m_stats; }
    void resetStats() { m_stats = {}; }
signals:
    void computed(const QVector<double> &result);
private:
    Stats m_stats;
};

