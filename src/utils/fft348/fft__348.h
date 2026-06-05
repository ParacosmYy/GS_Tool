/**
 * @file fft__348.h
 * @brief fft module fft__348
 */
#pragma once
#include <QObject>
#include <QVector>
class fft__348 : public QObject {
    Q_OBJECT
public:
    struct Stats { quint64 calls=0; quint64 items=0; quint64 errors=0; };
    explicit fft__348(QObject *p=nullptr) : QObject(p) {}
    ~fft__348() override = default;
    QVector<double> compute(const QVector<double> &input);
    Stats stats() const { return m_stats; }
    void resetStats() { m_stats = {}; }
signals:
    void computed(const QVector<double> &result);
private:
    Stats m_stats;
};

