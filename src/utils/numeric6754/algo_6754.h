/**
 * @file algo_6754.h
 * @brief Algorithm module 6754
 */
#pragma once
#include <QObject>
#include <QVector>
class algo_6754 : public QObject {
    Q_OBJECT
public:
    struct Stats { quint64 calls=0; quint64 items=0; quint64 errors=0; };
    explicit algo_6754(QObject *p=nullptr) : QObject(p) {}
    ~algo_6754() override = default;
    QVector<double> compute(const QVector<double> &input);
    Stats stats() const { return m_stats; }
    void resetStats() { m_stats = {}; }
signals:
    void computed(const QVector<double> &result);
private:
    Stats m_stats;
};
