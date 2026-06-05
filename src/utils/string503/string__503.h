/**
 * @file string__503.h
 * @brief string module string__503
 */
#pragma once
#include <QObject>
#include <QVector>
class string__503 : public QObject {
    Q_OBJECT
public:
    struct Stats { quint64 calls=0; quint64 items=0; quint64 errors=0; };
    explicit string__503(QObject *p=nullptr) : QObject(p) {}
    ~string__503() override = default;
    QVector<double> compute(const QVector<double> &input);
    Stats stats() const { return m_stats; }
    void resetStats() { m_stats = {}; }
signals:
    void computed(const QVector<double> &result);
private:
    Stats m_stats;
};

