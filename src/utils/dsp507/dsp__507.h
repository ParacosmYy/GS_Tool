/**
 * @file dsp__507.h
 * @brief dsp module dsp__507
 */
#pragma once
#include <QObject>
#include <QVector>
class dsp__507 : public QObject {
    Q_OBJECT
public:
    struct Stats { quint64 calls=0; quint64 items=0; quint64 errors=0; };
    explicit dsp__507(QObject *p=nullptr) : QObject(p) {}
    ~dsp__507() override = default;
    QVector<double> compute(const QVector<double> &input);
    Stats stats() const { return m_stats; }
    void resetStats() { m_stats = {}; }
signals:
    void computed(const QVector<double> &result);
private:
    Stats m_stats;
};

