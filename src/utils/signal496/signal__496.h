/**
 * @file signal__496.h
 * @brief signal module signal__496
 */
#pragma once
#include <QObject>
#include <QVector>
class signal__496 : public QObject {
    Q_OBJECT
public:
    struct Stats { quint64 calls=0; quint64 items=0; quint64 errors=0; };
    explicit signal__496(QObject *p=nullptr) : QObject(p) {}
    ~signal__496() override = default;
    QVector<double> compute(const QVector<double> &input);
    Stats stats() const { return m_stats; }
    void resetStats() { m_stats = {}; }
signals:
    void computed(const QVector<double> &result);
private:
    Stats m_stats;
};

