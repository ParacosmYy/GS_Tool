/**
 * @file interp__701.h
 * @brief interp module interp__701
 */
#pragma once
#include <QObject>
#include <QVector>
class interp__701 : public QObject {
    Q_OBJECT
public:
    struct Stats { quint64 calls=0; quint64 items=0; quint64 errors=0; };
    explicit interp__701(QObject *p=nullptr) : QObject(p) {}
    ~interp__701() override = default;
    QVector<double> compute(const QVector<double> &input);
    Stats stats() const { return m_stats; }
    void resetStats() { m_stats = {}; }
signals:
    void computed(const QVector<double> &result);
private:
    Stats m_stats;
};

