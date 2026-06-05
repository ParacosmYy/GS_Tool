/**
 * @file numeric__314.h
 * @brief numeric module numeric__314
 */
#pragma once
#include <QObject>
#include <QVector>
class numeric__314 : public QObject {
    Q_OBJECT
public:
    struct Stats { quint64 calls=0; quint64 items=0; quint64 errors=0; };
    explicit numeric__314(QObject *p=nullptr) : QObject(p) {}
    ~numeric__314() override = default;
    QVector<double> compute(const QVector<double> &input);
    Stats stats() const { return m_stats; }
    void resetStats() { m_stats = {}; }
signals:
    void computed(const QVector<double> &result);
private:
    Stats m_stats;
};

