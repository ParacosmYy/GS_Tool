/**
 * @file algo_922.h
 * @brief Algorithm module 922
 */
#pragma once
#include <QObject>
#include <QVector>
class algo_922 : public QObject {
    Q_OBJECT
public:
    struct Stats { quint64 calls=0; quint64 items=0; quint64 errors=0; };
    explicit algo_922(QObject *p=nullptr) : QObject(p) {}
    ~algo_922() override = default;
    QVector<double> compute(const QVector<double> &input);
    Stats stats() const { return m_stats; }
    void resetStats() { m_stats = {}; }
signals:
    void computed(const QVector<double> &result);
private:
    Stats m_stats;
};
