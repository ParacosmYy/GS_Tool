/**
 * @file numeric__664.h
 * @brief numeric module numeric__664
 */
#pragma once
#include <QObject>
#include <QVector>
class numeric__664 : public QObject {
    Q_OBJECT
public:
    struct Stats { quint64 calls=0; quint64 items=0; quint64 errors=0; };
    explicit numeric__664(QObject *p=nullptr) : QObject(p) {}
    ~numeric__664() override = default;
    QVector<double> compute(const QVector<double> &input);
    Stats stats() const { return m_stats; }
    void resetStats() { m_stats = {}; }
signals:
    void computed(const QVector<double> &result);
private:
    Stats m_stats;
};

