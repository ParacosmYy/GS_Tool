/**
 * @file string__343.h
 * @brief string module string__343
 */
#pragma once
#include <QObject>
#include <QVector>
class string__343 : public QObject {
    Q_OBJECT
public:
    struct Stats { quint64 calls=0; quint64 items=0; quint64 errors=0; };
    explicit string__343(QObject *p=nullptr) : QObject(p) {}
    ~string__343() override = default;
    QVector<double> compute(const QVector<double> &input);
    Stats stats() const { return m_stats; }
    void resetStats() { m_stats = {}; }
signals:
    void computed(const QVector<double> &result);
private:
    Stats m_stats;
};

