/**
 * @file MedianFilter3.h
 * @brief graph algorithm module - MedianFilter3
 */
#pragma once
#include <QObject>
#include <QVector>
class MedianFilter3 : public QObject {
    Q_OBJECT
public:
    struct Stats { quint64 calls = 0; quint64 items = 0; quint64 errors = 0; };
    explicit MedianFilter3(QObject *p = nullptr) : QObject(p) {}
    ~MedianFilter3() override = default;
    QVector<double> compute(const QVector<double> &input);
    Stats stats() const { return m_stats; }
    void resetStats() { m_stats = {}; }
signals:
    void computed(const QVector<double> &result);
private:
    Stats m_stats;
};

