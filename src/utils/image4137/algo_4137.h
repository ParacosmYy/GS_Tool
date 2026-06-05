/**
 * @file algo_4137.h
 * @brief Algorithm module 4137
 */
#pragma once
#include <QObject>
#include <QVector>
class algo_4137 : public QObject {
    Q_OBJECT
public:
    struct Stats { quint64 calls=0; quint64 items=0; quint64 errors=0; };
    explicit algo_4137(QObject *p=nullptr) : QObject(p) {}
    ~algo_4137() override = default;
    QVector<double> compute(const QVector<double> &input);
    Stats stats() const { return m_stats; }
    void resetStats() { m_stats = {}; }
signals:
    void computed(const QVector<double> &result);
private:
    Stats m_stats;
};
