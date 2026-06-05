/**
 * @file optim__585.h
 * @brief optim module optim__585
 */
#pragma once
#include <QObject>
#include <QVector>
class optim__585 : public QObject {
    Q_OBJECT
public:
    struct Stats { quint64 calls=0; quint64 items=0; quint64 errors=0; };
    explicit optim__585(QObject *p=nullptr) : QObject(p) {}
    ~optim__585() override = default;
    QVector<double> compute(const QVector<double> &input);
    Stats stats() const { return m_stats; }
    void resetStats() { m_stats = {}; }
signals:
    void computed(const QVector<double> &result);
private:
    Stats m_stats;
};

