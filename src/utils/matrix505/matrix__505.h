/**
 * @file matrix__505.h
 * @brief matrix module matrix__505
 */
#pragma once
#include <QObject>
#include <QVector>
class matrix__505 : public QObject {
    Q_OBJECT
public:
    struct Stats { quint64 calls=0; quint64 items=0; quint64 errors=0; };
    explicit matrix__505(QObject *p=nullptr) : QObject(p) {}
    ~matrix__505() override = default;
    QVector<double> compute(const QVector<double> &input);
    Stats stats() const { return m_stats; }
    void resetStats() { m_stats = {}; }
signals:
    void computed(const QVector<double> &result);
private:
    Stats m_stats;
};

