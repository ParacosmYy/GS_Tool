/**
 * @file LDPCCode2.h
 * @brief quantum algorithm module - LDPCCode2
 */
#pragma once
#include <QObject>
#include <QVector>
class LDPCCode2 : public QObject {
    Q_OBJECT
public:
    struct Stats { quint64 calls = 0; quint64 items = 0; quint64 errors = 0; };
    explicit LDPCCode2(QObject *p = nullptr) : QObject(p) {}
    ~LDPCCode2() override = default;
    QVector<double> compute(const QVector<double> &input);
    Stats stats() const { return m_stats; }
    void resetStats() { m_stats = {}; }
signals:
    void computed(const QVector<double> &result);
private:
    Stats m_stats;
};

