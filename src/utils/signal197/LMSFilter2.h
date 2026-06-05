/**
 * @file LMSFilter2.h
 * @brief signal algorithm module - LMSFilter2
 */
#pragma once
#include <QObject>
#include <QVector>
class LMSFilter2 : public QObject {
    Q_OBJECT
public:
    struct Stats { quint64 calls = 0; quint64 items = 0; quint64 errors = 0; };
    explicit LMSFilter2(QObject *p = nullptr) : QObject(p) {}
    ~LMSFilter2() override = default;
    QVector<double> compute(const QVector<double> &input);
    Stats stats() const { return m_stats; }
    void resetStats() { m_stats = {}; }
signals:
    void computed(const QVector<double> &result);
private:
    Stats m_stats;
};

