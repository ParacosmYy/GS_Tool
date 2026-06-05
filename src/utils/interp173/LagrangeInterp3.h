/**
 * @file LagrangeInterp3.h
 * @brief Lagrange polynomial interpolation
 */
#pragma once
#include <QObject>
#include <QVector>
#include <QByteArray>

/**
 * @brief Lagrange polynomial interpolation
 */
class LagrangeInterp3 : public QObject {
    Q_OBJECT
public:
    struct Stats { quint64 calls = 0; quint64 items = 0; quint64 errors = 0; };
    explicit LagrangeInterp3(QObject *p = nullptr) : QObject(p) {}
    ~LagrangeInterp3() override = default;
    QVector<double> compute(const QVector<double> &input);
    Stats stats() const { return m_stats; }
    void resetStats() { m_stats = {}; }
signals:
    void computed(const QVector<double> &result);
private:
    Stats m_stats;
};

