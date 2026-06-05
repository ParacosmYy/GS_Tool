/**
 * @file HierCluster2.h
 * @brief interp algorithm module - HierCluster2
 */
#pragma once
#include <QObject>
#include <QVector>
class HierCluster2 : public QObject {
    Q_OBJECT
public:
    struct Stats { quint64 calls = 0; quint64 items = 0; quint64 errors = 0; };
    explicit HierCluster2(QObject *p = nullptr) : QObject(p) {}
    ~HierCluster2() override = default;
    QVector<double> compute(const QVector<double> &input);
    Stats stats() const { return m_stats; }
    void resetStats() { m_stats = {}; }
signals:
    void computed(const QVector<double> &result);
private:
    Stats m_stats;
};

