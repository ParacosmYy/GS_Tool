/**
 * @file algo_4089.h
 * @brief Algorithm module 4089
 */
#pragma once
#include <QObject>
#include <QVector>
class algo_4089 : public QObject {
    Q_OBJECT
public:
    struct Stats { quint64 calls=0; quint64 items=0; quint64 errors=0; };
    explicit algo_4089(QObject *p=nullptr) : QObject(p) {}
    ~algo_4089() override = default;
    QVector<double> compute(const QVector<double> &input);
    Stats stats() const { return m_stats; }
    void resetStats() { m_stats = {}; }
signals:
    void computed(const QVector<double> &result);
private:
    Stats m_stats;
};
