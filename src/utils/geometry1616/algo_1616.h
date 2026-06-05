/**
 * @file algo_1616.h
 * @brief Algorithm module 1616
 */
#pragma once
#include <QObject>
#include <QVector>
class algo_1616 : public QObject {
    Q_OBJECT
public:
    struct Stats { quint64 calls=0; quint64 items=0; quint64 errors=0; };
    explicit algo_1616(QObject *p=nullptr) : QObject(p) {}
    ~algo_1616() override = default;
    QVector<double> compute(const QVector<double> &input);
    Stats stats() const { return m_stats; }
    void resetStats() { m_stats = {}; }
signals:
    void computed(const QVector<double> &result);
private:
    Stats m_stats;
};
