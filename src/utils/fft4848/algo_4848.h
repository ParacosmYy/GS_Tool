/**
 * @file algo_4848.h
 * @brief Algorithm module 4848
 */
#pragma once
#include <QObject>
#include <QVector>
class algo_4848 : public QObject {
    Q_OBJECT
public:
    struct Stats { quint64 calls=0; quint64 items=0; quint64 errors=0; };
    explicit algo_4848(QObject *p=nullptr) : QObject(p) {}
    ~algo_4848() override = default;
    QVector<double> compute(const QVector<double> &input);
    Stats stats() const { return m_stats; }
    void resetStats() { m_stats = {}; }
signals:
    void computed(const QVector<double> &result);
private:
    Stats m_stats;
};
