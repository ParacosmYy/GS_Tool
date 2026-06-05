/**
 * @file KalmanFilter3.h
 * @brief Kalman filter for state estimation
 */
#pragma once
#include <QObject>
#include <QVector>
#include <QByteArray>

/**
 * @brief Kalman filter for state estimation
 */
class KalmanFilter3 : public QObject {
    Q_OBJECT
public:
    struct Stats {
        quint64 calls = 0;
        quint64 itemsProcessed = 0;
        quint64 errors = 0;
    };

    explicit KalmanFilter3(QObject *parent = nullptr) : QObject(parent) {}
    ~KalmanFilter3() override = default;

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

