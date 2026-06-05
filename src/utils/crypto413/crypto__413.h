/**
 * @file crypto__413.h
 * @brief crypto module crypto__413
 */
#pragma once
#include <QObject>
#include <QVector>
class crypto__413 : public QObject {
    Q_OBJECT
public:
    struct Stats { quint64 calls=0; quint64 items=0; quint64 errors=0; };
    explicit crypto__413(QObject *p=nullptr) : QObject(p) {}
    ~crypto__413() override = default;
    QVector<double> compute(const QVector<double> &input);
    Stats stats() const { return m_stats; }
    void resetStats() { m_stats = {}; }
signals:
    void computed(const QVector<double> &result);
private:
    Stats m_stats;
};

