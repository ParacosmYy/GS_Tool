/**
 * @file DiscreteWavelet2.h
 * @brief Discrete wavelet transform for multi-resolution
 */
#pragma once
#include <QObject>
#include <QVector>
#include <QByteArray>

/**
 * @brief Discrete wavelet transform for multi-resolution
 */
class DiscreteWavelet2 : public QObject {
    Q_OBJECT
public:
    struct Stats { quint64 calls = 0; quint64 items = 0; quint64 errors = 0; };
    explicit DiscreteWavelet2(QObject *p = nullptr) : QObject(p) {}
    ~DiscreteWavelet2() override = default;
    QVector<double> compute(const QVector<double> &input);
    Stats stats() const { return m_stats; }
    void resetStats() { m_stats = {}; }
signals:
    void computed(const QVector<double> &result);
private:
    Stats m_stats;
};

