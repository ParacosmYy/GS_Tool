/**
 * @file algo_3053.h
 * @brief Algorithm module 3053
 */
#pragma once
#include <QObject>
#include <QVector>
class algo_3053 : public QObject {
    Q_OBJECT
public:
    struct Stats { quint64 calls=0; quint64 items=0; quint64 errors=0; };
    explicit algo_3053(QObject *p=nullptr) : QObject(p) {}
    ~algo_3053() override = default;
    QVector<double> compute(const QVector<double> &input);
    Stats stats() const { return m_stats; }
    void resetStats() { m_stats = {}; }
signals:
    void computed(const QVector<double> &result);
private:
    Stats m_stats;
};
