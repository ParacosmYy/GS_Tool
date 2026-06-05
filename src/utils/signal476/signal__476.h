/**
 * @file signal__476.h
 * @brief signal module signal__476
 */
#pragma once
#include <QObject>
#include <QVector>
class signal__476 : public QObject {
    Q_OBJECT
public:
    struct Stats { quint64 calls=0; quint64 items=0; quint64 errors=0; };
    explicit signal__476(QObject *p=nullptr) : QObject(p) {}
    ~signal__476() override = default;
    QVector<double> compute(const QVector<double> &input);
    Stats stats() const { return m_stats; }
    void resetStats() { m_stats = {}; }
signals:
    void computed(const QVector<double> &result);
private:
    Stats m_stats;
};

