/**
 * @file BLAKE2Hash2.h
 * @brief optim algorithm module - BLAKE2Hash2
 */
#pragma once
#include <QObject>
#include <QVector>
class BLAKE2Hash2 : public QObject {
    Q_OBJECT
public:
    struct Stats { quint64 calls = 0; quint64 items = 0; quint64 errors = 0; };
    explicit BLAKE2Hash2(QObject *p = nullptr) : QObject(p) {}
    ~BLAKE2Hash2() override = default;
    QVector<double> compute(const QVector<double> &input);
    Stats stats() const { return m_stats; }
    void resetStats() { m_stats = {}; }
signals:
    void computed(const QVector<double> &result);
private:
    Stats m_stats;
};

