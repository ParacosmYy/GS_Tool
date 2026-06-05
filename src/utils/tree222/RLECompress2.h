/**
 * @file RLECompress2.h
 * @brief tree algorithm module - RLECompress2
 */
#pragma once
#include <QObject>
#include <QVector>
class RLECompress2 : public QObject {
    Q_OBJECT
public:
    struct Stats { quint64 calls = 0; quint64 items = 0; quint64 errors = 0; };
    explicit RLECompress2(QObject *p = nullptr) : QObject(p) {}
    ~RLECompress2() override = default;
    QVector<double> compute(const QVector<double> &input);
    Stats stats() const { return m_stats; }
    void resetStats() { m_stats = {}; }
signals:
    void computed(const QVector<double> &result);
private:
    Stats m_stats;
};

