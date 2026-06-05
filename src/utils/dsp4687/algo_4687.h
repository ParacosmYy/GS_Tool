/**
 * @file algo_4687.h
 * @brief Algorithm module 4687
 */
#pragma once
#include <QObject>
#include <QVector>
class algo_4687 : public QObject {
    Q_OBJECT
public:
    struct Stats { quint64 calls=0; quint64 items=0; quint64 errors=0; };
    explicit algo_4687(QObject *p=nullptr) : QObject(p) {}
    ~algo_4687() override = default;
    QVector<double> compute(const QVector<double> &input);
    Stats stats() const { return m_stats; }
    void resetStats() { m_stats = {}; }
signals:
    void computed(const QVector<double> &result);
private:
    Stats m_stats;
};
