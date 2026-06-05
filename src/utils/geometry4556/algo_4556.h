/**
 * @file algo_4556.h
 * @brief Algorithm module 4556
 */
#pragma once
#include <QObject>
#include <QVector>
class algo_4556 : public QObject {
    Q_OBJECT
public:
    struct Stats { quint64 calls=0; quint64 items=0; quint64 errors=0; };
    explicit algo_4556(QObject *p=nullptr) : QObject(p) {}
    ~algo_4556() override = default;
    QVector<double> compute(const QVector<double> &input);
    Stats stats() const { return m_stats; }
    void resetStats() { m_stats = {}; }
signals:
    void computed(const QVector<double> &result);
private:
    Stats m_stats;
};
