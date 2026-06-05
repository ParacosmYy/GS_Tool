/**
 * @file numeric__364.h
 * @brief numeric module numeric__364
 */
#pragma once
#include <QObject>
#include <QVector>
class numeric__364 : public QObject {
    Q_OBJECT
public:
    struct Stats { quint64 calls=0; quint64 items=0; quint64 errors=0; };
    explicit numeric__364(QObject *p=nullptr) : QObject(p) {}
    ~numeric__364() override = default;
    QVector<double> compute(const QVector<double> &input);
    Stats stats() const { return m_stats; }
    void resetStats() { m_stats = {}; }
signals:
    void computed(const QVector<double> &result);
private:
    Stats m_stats;
};

