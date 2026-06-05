/**
 * @file BCHCode2.h
 * @brief neural algorithm module - BCHCode2
 */
#pragma once
#include <QObject>
#include <QVector>
class BCHCode2 : public QObject {
    Q_OBJECT
public:
    struct Stats { quint64 calls = 0; quint64 items = 0; quint64 errors = 0; };
    explicit BCHCode2(QObject *p = nullptr) : QObject(p) {}
    ~BCHCode2() override = default;
    QVector<double> compute(const QVector<double> &input);
    Stats stats() const { return m_stats; }
    void resetStats() { m_stats = {}; }
signals:
    void computed(const QVector<double> &result);
private:
    Stats m_stats;
};

