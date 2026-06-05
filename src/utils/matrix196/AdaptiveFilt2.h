/**
 * @file AdaptiveFilt2.h
 * @brief matrix algorithm module - AdaptiveFilt2
 */
#pragma once
#include <QObject>
#include <QVector>
class AdaptiveFilt2 : public QObject {
    Q_OBJECT
public:
    struct Stats { quint64 calls = 0; quint64 items = 0; quint64 errors = 0; };
    explicit AdaptiveFilt2(QObject *p = nullptr) : QObject(p) {}
    ~AdaptiveFilt2() override = default;
    QVector<double> compute(const QVector<double> &input);
    Stats stats() const { return m_stats; }
    void resetStats() { m_stats = {}; }
signals:
    void computed(const QVector<double> &result);
private:
    Stats m_stats;
};

