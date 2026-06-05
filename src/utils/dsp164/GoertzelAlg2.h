/**
 * @file GoertzelAlg2.h
 * @brief Goertzel algorithm for single-frequency DFT
 */
#pragma once
#include <QObject>
#include <QVector>
#include <QByteArray>

/**
 * @brief Goertzel algorithm for single-frequency DFT
 */
class GoertzelAlg2 : public QObject {
    Q_OBJECT
public:
    struct Stats {
        quint64 calls = 0;
        quint64 itemsProcessed = 0;
        quint64 errors = 0;
    };

    explicit GoertzelAlg2(QObject *parent = nullptr) : QObject(parent) {}
    ~GoertzelAlg2() override = default;

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

