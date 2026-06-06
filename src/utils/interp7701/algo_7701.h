/**
 * @file algo_7701.h
 * @brief Algorithm module 7701
 */
#pragma once
#include <QObject>
#include <QVector>
class algo_7701 : public QObject {
    Q_OBJECT
public:
    struct Stats { quint64 calls=0; quint64 items=0; quint64 errors=0; };
    explicit algo_7701(QObject *p=nullptr) : QObject(p) {}
    ~algo_7701() override = default;
    QVector<double> compute(const QVector<double> &input);
    Stats stats() const { return m_stats; }
    void resetStats() { m_stats = {}; }
signals:
    void computed(const QVector<double> &result);
private:
    Stats m_stats;
};
