/**
 * @file dsp__497.h
 * @brief dsp module dsp__497
 */
#pragma once
#include <QObject>
#include <QVector>
class dsp__497 : public QObject {
    Q_OBJECT
public:
    struct Stats { quint64 calls=0; quint64 items=0; quint64 errors=0; };
    explicit dsp__497(QObject *p=nullptr) : QObject(p) {}
    ~dsp__497() override = default;
    QVector<double> compute(const QVector<double> &input);
    Stats stats() const { return m_stats; }
    void resetStats() { m_stats = {}; }
signals:
    void computed(const QVector<double> &result);
private:
    Stats m_stats;
};

