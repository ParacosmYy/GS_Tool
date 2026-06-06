/**
 * @file algo_7560.h
 * @brief Algorithm module 7560
 */
#pragma once
#include <QObject>
#include <QVector>
class algo_7560 : public QObject {
    Q_OBJECT
public:
    struct Stats { quint64 calls=0; quint64 items=0; quint64 errors=0; };
    explicit algo_7560(QObject *p=nullptr) : QObject(p) {}
    ~algo_7560() override = default;
    QVector<double> compute(const QVector<double> &input);
    Stats stats() const { return m_stats; }
    void resetStats() { m_stats = {}; }
signals:
    void computed(const QVector<double> &result);
private:
    Stats m_stats;
};
