/**
 * @file fft__608.h
 * @brief fft module fft__608
 */
#pragma once
#include <QObject>
#include <QVector>
class fft__608 : public QObject {
    Q_OBJECT
public:
    struct Stats { quint64 calls=0; quint64 items=0; quint64 errors=0; };
    explicit fft__608(QObject *p=nullptr) : QObject(p) {}
    ~fft__608() override = default;
    QVector<double> compute(const QVector<double> &input);
    Stats stats() const { return m_stats; }
    void resetStats() { m_stats = {}; }
signals:
    void computed(const QVector<double> &result);
private:
    Stats m_stats;
};

