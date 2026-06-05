/**
 * @file dsp__307.h
 * @brief dsp module dsp__307
 */
#pragma once
#include <QObject>
#include <QVector>
class dsp__307 : public QObject {
    Q_OBJECT
public:
    struct Stats { quint64 calls=0; quint64 items=0; quint64 errors=0; };
    explicit dsp__307(QObject *p=nullptr) : QObject(p) {}
    ~dsp__307() override = default;
    QVector<double> compute(const QVector<double> &input);
    Stats stats() const { return m_stats; }
    void resetStats() { m_stats = {}; }
signals:
    void computed(const QVector<double> &result);
private:
    Stats m_stats;
};

