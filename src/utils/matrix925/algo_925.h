/**
 * @file algo_925.h
 * @brief Algorithm module 925
 */
#pragma once
#include <QObject>
#include <QVector>
class algo_925 : public QObject {
    Q_OBJECT
public:
    struct Stats { quint64 calls=0; quint64 items=0; quint64 errors=0; };
    explicit algo_925(QObject *p=nullptr) : QObject(p) {}
    ~algo_925() override = default;
    QVector<double> compute(const QVector<double> &input);
    Stats stats() const { return m_stats; }
    void resetStats() { m_stats = {}; }
signals:
    void computed(const QVector<double> &result);
private:
    Stats m_stats;
};
