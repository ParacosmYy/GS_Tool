/**
 * @file algo_2165.h
 * @brief Algorithm module 2165
 */
#pragma once
#include <QObject>
#include <QVector>
class algo_2165 : public QObject {
    Q_OBJECT
public:
    struct Stats { quint64 calls=0; quint64 items=0; quint64 errors=0; };
    explicit algo_2165(QObject *p=nullptr) : QObject(p) {}
    ~algo_2165() override = default;
    QVector<double> compute(const QVector<double> &input);
    Stats stats() const { return m_stats; }
    void resetStats() { m_stats = {}; }
signals:
    void computed(const QVector<double> &result);
private:
    Stats m_stats;
};
