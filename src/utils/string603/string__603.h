/**
 * @file string__603.h
 * @brief string module string__603
 */
#pragma once
#include <QObject>
#include <QVector>
class string__603 : public QObject {
    Q_OBJECT
public:
    struct Stats { quint64 calls=0; quint64 items=0; quint64 errors=0; };
    explicit string__603(QObject *p=nullptr) : QObject(p) {}
    ~string__603() override = default;
    QVector<double> compute(const QVector<double> &input);
    Stats stats() const { return m_stats; }
    void resetStats() { m_stats = {}; }
signals:
    void computed(const QVector<double> &result);
private:
    Stats m_stats;
};

