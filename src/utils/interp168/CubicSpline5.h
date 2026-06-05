/**
 * @file CubicSpline5.h
 * @brief Cubic spline interpolation for smooth curve fitting
 */
#pragma once
#include <QObject>
#include <QVector>
#include <QByteArray>

/**
 * @brief Cubic spline interpolation for smooth curve fitting
 */
class CubicSpline5 : public QObject {
    Q_OBJECT
public:
    struct Stats {
        quint64 calls = 0;
        quint64 itemsProcessed = 0;
        quint64 errors = 0;
    };

    explicit CubicSpline5(QObject *parent = nullptr) : QObject(parent) {}
    ~CubicSpline5() override = default;

    /** @brief Process input data */
    QVector<double> compute(const QVector<double> &input);

    /** @brief Get statistics */
    Stats stats() const { return m_stats; }

    /** @brief Reset statistics */
    void resetStats() { m_stats = {}; }

signals:
    void computed(const QVector<double> &result);

private:
    Stats m_stats;
};

