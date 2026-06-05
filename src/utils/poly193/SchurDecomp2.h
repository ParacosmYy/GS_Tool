/**
 * @file SchurDecomp2.h
 * @brief poly algorithm module - SchurDecomp2
 */
#pragma once
#include <QObject>
#include <QVector>
class SchurDecomp2 : public QObject {
    Q_OBJECT
public:
    struct Stats { quint64 calls = 0; quint64 items = 0; quint64 errors = 0; };
    explicit SchurDecomp2(QObject *p = nullptr) : QObject(p) {}
    ~SchurDecomp2() override = default;
    QVector<double> compute(const QVector<double> &input);
    Stats stats() const { return m_stats; }
    void resetStats() { m_stats = {}; }
signals:
    void computed(const QVector<double> &result);
private:
    Stats m_stats;
};

