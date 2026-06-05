/**
 * @file string__723.h
 * @brief string module string__723
 */
#pragma once
#include <QObject>
#include <QVector>
class string__723 : public QObject {
    Q_OBJECT
public:
    struct Stats { quint64 calls=0; quint64 items=0; quint64 errors=0; };
    explicit string__723(QObject *p=nullptr) : QObject(p) {}
    ~string__723() override = default;
    QVector<double> compute(const QVector<double> &input);
    Stats stats() const { return m_stats; }
    void resetStats() { m_stats = {}; }
signals:
    void computed(const QVector<double> &result);
private:
    Stats m_stats;
};

