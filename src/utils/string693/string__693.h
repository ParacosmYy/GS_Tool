/**
 * @file string__693.h
 * @brief string module string__693
 */
#pragma once
#include <QObject>
#include <QVector>
class string__693 : public QObject {
    Q_OBJECT
public:
    struct Stats { quint64 calls=0; quint64 items=0; quint64 errors=0; };
    explicit string__693(QObject *p=nullptr) : QObject(p) {}
    ~string__693() override = default;
    QVector<double> compute(const QVector<double> &input);
    Stats stats() const { return m_stats; }
    void resetStats() { m_stats = {}; }
signals:
    void computed(const QVector<double> &result);
private:
    Stats m_stats;
};

