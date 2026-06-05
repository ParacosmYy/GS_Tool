/**
 * @file HeapSort4.h
 * @brief sort algorithm module - HeapSort4
 */
#pragma once
#include <QObject>
#include <QVector>
class HeapSort4 : public QObject {
    Q_OBJECT
public:
    struct Stats { quint64 calls = 0; quint64 items = 0; quint64 errors = 0; };
    explicit HeapSort4(QObject *p = nullptr) : QObject(p) {}
    ~HeapSort4() override = default;
    QVector<double> compute(const QVector<double> &input);
    Stats stats() const { return m_stats; }
    void resetStats() { m_stats = {}; }
signals:
    void computed(const QVector<double> &result);
private:
    Stats m_stats;
};

