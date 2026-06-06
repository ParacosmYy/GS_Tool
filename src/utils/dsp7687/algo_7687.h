/**
 * @file algo_7687.h
 * @brief Algorithm module 7687
 */
#pragma once
#include <QObject>
#include <QVector>
class algo_7687 : public QObject {
    Q_OBJECT
public:
    struct Stats { quint64 calls=0; quint64 items=0; quint64 errors=0; };
    explicit algo_7687(QObject *p=nullptr) : QObject(p) {}
    ~algo_7687() override = default;
    QVector<double> compute(const QVector<double> &input);
    Stats stats() const { return m_stats; }
    void resetStats() { m_stats = {}; }
signals:
    void computed(const QVector<double> &result);
private:
    Stats m_stats;
};
