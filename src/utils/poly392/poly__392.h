/**
 * @file poly__392.h
 * @brief poly module poly__392
 */
#pragma once
#include <QObject>
#include <QVector>
class poly__392 : public QObject {
    Q_OBJECT
public:
    struct Stats { quint64 calls=0; quint64 items=0; quint64 errors=0; };
    explicit poly__392(QObject *p=nullptr) : QObject(p) {}
    ~poly__392() override = default;
    QVector<double> compute(const QVector<double> &input);
    Stats stats() const { return m_stats; }
    void resetStats() { m_stats = {}; }
signals:
    void computed(const QVector<double> &result);
private:
    Stats m_stats;
};

