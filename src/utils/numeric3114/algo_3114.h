/**
 * @file algo_3114.h
 * @brief Algorithm module 3114
 */
#pragma once
#include <QObject>
#include <QVector>
class algo_3114 : public QObject {
    Q_OBJECT
public:
    struct Stats { quint64 calls=0; quint64 items=0; quint64 errors=0; };
    explicit algo_3114(QObject *p=nullptr) : QObject(p) {}
    ~algo_3114() override = default;
    QVector<double> compute(const QVector<double> &input);
    Stats stats() const { return m_stats; }
    void resetStats() { m_stats = {}; }
signals:
    void computed(const QVector<double> &result);
private:
    Stats m_stats;
};
