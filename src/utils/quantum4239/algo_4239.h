/**
 * @file algo_4239.h
 * @brief Algorithm module 4239
 */
#pragma once
#include <QObject>
#include <QVector>
class algo_4239 : public QObject {
    Q_OBJECT
public:
    struct Stats { quint64 calls=0; quint64 items=0; quint64 errors=0; };
    explicit algo_4239(QObject *p=nullptr) : QObject(p) {}
    ~algo_4239() override = default;
    QVector<double> compute(const QVector<double> &input);
    Stats stats() const { return m_stats; }
    void resetStats() { m_stats = {}; }
signals:
    void computed(const QVector<double> &result);
private:
    Stats m_stats;
};
