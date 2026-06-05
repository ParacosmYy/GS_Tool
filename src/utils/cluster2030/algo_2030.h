/**
 * @file algo_2030.h
 * @brief Algorithm module 2030
 */
#pragma once
#include <QObject>
#include <QVector>
class algo_2030 : public QObject {
    Q_OBJECT
public:
    struct Stats { quint64 calls=0; quint64 items=0; quint64 errors=0; };
    explicit algo_2030(QObject *p=nullptr) : QObject(p) {}
    ~algo_2030() override = default;
    QVector<double> compute(const QVector<double> &input);
    Stats stats() const { return m_stats; }
    void resetStats() { m_stats = {}; }
signals:
    void computed(const QVector<double> &result);
private:
    Stats m_stats;
};
