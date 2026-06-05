/**
 * @file fft__428.h
 * @brief fft module fft__428
 */
#pragma once
#include <QObject>
#include <QVector>
class fft__428 : public QObject {
    Q_OBJECT
public:
    struct Stats { quint64 calls=0; quint64 items=0; quint64 errors=0; };
    explicit fft__428(QObject *p=nullptr) : QObject(p) {}
    ~fft__428() override = default;
    QVector<double> compute(const QVector<double> &input);
    Stats stats() const { return m_stats; }
    void resetStats() { m_stats = {}; }
signals:
    void computed(const QVector<double> &result);
private:
    Stats m_stats;
};

