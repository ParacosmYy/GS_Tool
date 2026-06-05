/**
 * @file algo_6457.h
 * @brief Algorithm module 6457
 */
#pragma once
#include <QObject>
#include <QVector>
class algo_6457 : public QObject {
    Q_OBJECT
public:
    struct Stats { quint64 calls=0; quint64 items=0; quint64 errors=0; };
    explicit algo_6457(QObject *p=nullptr) : QObject(p) {}
    ~algo_6457() override = default;
    QVector<double> compute(const QVector<double> &input);
    Stats stats() const { return m_stats; }
    void resetStats() { m_stats = {}; }
signals:
    void computed(const QVector<double> &result);
private:
    Stats m_stats;
};
