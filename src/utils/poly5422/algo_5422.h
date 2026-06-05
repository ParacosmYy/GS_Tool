/**
 * @file algo_5422.h
 * @brief Algorithm module 5422
 */
#pragma once
#include <QObject>
#include <QVector>
class algo_5422 : public QObject {
    Q_OBJECT
public:
    struct Stats { quint64 calls=0; quint64 items=0; quint64 errors=0; };
    explicit algo_5422(QObject *p=nullptr) : QObject(p) {}
    ~algo_5422() override = default;
    QVector<double> compute(const QVector<double> &input);
    Stats stats() const { return m_stats; }
    void resetStats() { m_stats = {}; }
signals:
    void computed(const QVector<double> &result);
private:
    Stats m_stats;
};
