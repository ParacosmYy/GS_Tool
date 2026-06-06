/**
 * @file algo_7164.h
 * @brief Algorithm module 7164
 */
#pragma once
#include <QObject>
#include <QVector>
class algo_7164 : public QObject {
    Q_OBJECT
public:
    struct Stats { quint64 calls=0; quint64 items=0; quint64 errors=0; };
    explicit algo_7164(QObject *p=nullptr) : QObject(p) {}
    ~algo_7164() override = default;
    QVector<double> compute(const QVector<double> &input);
    Stats stats() const { return m_stats; }
    void resetStats() { m_stats = {}; }
signals:
    void computed(const QVector<double> &result);
private:
    Stats m_stats;
};
