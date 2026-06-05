/**
 * @file algo_3606.h
 * @brief Algorithm module 3606
 */
#pragma once
#include <QObject>
#include <QVector>
class algo_3606 : public QObject {
    Q_OBJECT
public:
    struct Stats { quint64 calls=0; quint64 items=0; quint64 errors=0; };
    explicit algo_3606(QObject *p=nullptr) : QObject(p) {}
    ~algo_3606() override = default;
    QVector<double> compute(const QVector<double> &input);
    Stats stats() const { return m_stats; }
    void resetStats() { m_stats = {}; }
signals:
    void computed(const QVector<double> &result);
private:
    Stats m_stats;
};
