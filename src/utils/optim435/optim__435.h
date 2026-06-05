/**
 * @file optim__435.h
 * @brief optim module optim__435
 */
#pragma once
#include <QObject>
#include <QVector>
class optim__435 : public QObject {
    Q_OBJECT
public:
    struct Stats { quint64 calls=0; quint64 items=0; quint64 errors=0; };
    explicit optim__435(QObject *p=nullptr) : QObject(p) {}
    ~optim__435() override = default;
    QVector<double> compute(const QVector<double> &input);
    Stats stats() const { return m_stats; }
    void resetStats() { m_stats = {}; }
signals:
    void computed(const QVector<double> &result);
private:
    Stats m_stats;
};

