/**
 * @file poly__372.h
 * @brief poly module poly__372
 */
#pragma once
#include <QObject>
#include <QVector>
class poly__372 : public QObject {
    Q_OBJECT
public:
    struct Stats { quint64 calls=0; quint64 items=0; quint64 errors=0; };
    explicit poly__372(QObject *p=nullptr) : QObject(p) {}
    ~poly__372() override = default;
    QVector<double> compute(const QVector<double> &input);
    Stats stats() const { return m_stats; }
    void resetStats() { m_stats = {}; }
signals:
    void computed(const QVector<double> &result);
private:
    Stats m_stats;
};

