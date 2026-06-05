/**
 * @file algo_1502.h
 * @brief Algorithm module 1502
 */
#pragma once
#include <QObject>
#include <QVector>
class algo_1502 : public QObject {
    Q_OBJECT
public:
    struct Stats { quint64 calls=0; quint64 items=0; quint64 errors=0; };
    explicit algo_1502(QObject *p=nullptr) : QObject(p) {}
    ~algo_1502() override = default;
    QVector<double> compute(const QVector<double> &input);
    Stats stats() const { return m_stats; }
    void resetStats() { m_stats = {}; }
signals:
    void computed(const QVector<double> &result);
private:
    Stats m_stats;
};
