/**
 * @file OPTICS2.h
 * @brief poly algorithm module - OPTICS2
 */
#pragma once
#include <QObject>
#include <QVector>
class OPTICS2 : public QObject {
    Q_OBJECT
public:
    struct Stats { quint64 calls = 0; quint64 items = 0; quint64 errors = 0; };
    explicit OPTICS2(QObject *p = nullptr) : QObject(p) {}
    ~OPTICS2() override = default;
    QVector<double> compute(const QVector<double> &input);
    Stats stats() const { return m_stats; }
    void resetStats() { m_stats = {}; }
signals:
    void computed(const QVector<double> &result);
private:
    Stats m_stats;
};

