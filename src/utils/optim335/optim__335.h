/**
 * @file optim__335.h
 * @brief optim module optim__335
 */
#pragma once
#include <QObject>
#include <QVector>
class optim__335 : public QObject {
    Q_OBJECT
public:
    struct Stats { quint64 calls=0; quint64 items=0; quint64 errors=0; };
    explicit optim__335(QObject *p=nullptr) : QObject(p) {}
    ~optim__335() override = default;
    QVector<double> compute(const QVector<double> &input);
    Stats stats() const { return m_stats; }
    void resetStats() { m_stats = {}; }
signals:
    void computed(const QVector<double> &result);
private:
    Stats m_stats;
};

