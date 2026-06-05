/**
 * @file algo_2125.h
 * @brief Algorithm module 2125
 */
#pragma once
#include <QObject>
#include <QVector>
class algo_2125 : public QObject {
    Q_OBJECT
public:
    struct Stats { quint64 calls=0; quint64 items=0; quint64 errors=0; };
    explicit algo_2125(QObject *p=nullptr) : QObject(p) {}
    ~algo_2125() override = default;
    QVector<double> compute(const QVector<double> &input);
    Stats stats() const { return m_stats; }
    void resetStats() { m_stats = {}; }
signals:
    void computed(const QVector<double> &result);
private:
    Stats m_stats;
};
