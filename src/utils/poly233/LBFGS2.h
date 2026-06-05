/**
 * @file LBFGS2.h
 * @brief poly algorithm module - LBFGS2
 */
#pragma once
#include <QObject>
#include <QVector>
class LBFGS2 : public QObject {
    Q_OBJECT
public:
    struct Stats { quint64 calls = 0; quint64 items = 0; quint64 errors = 0; };
    explicit LBFGS2(QObject *p = nullptr) : QObject(p) {}
    ~LBFGS2() override = default;
    QVector<double> compute(const QVector<double> &input);
    Stats stats() const { return m_stats; }
    void resetStats() { m_stats = {}; }
signals:
    void computed(const QVector<double> &result);
private:
    Stats m_stats;
};

