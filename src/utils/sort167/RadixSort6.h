/**
 * @file RadixSort6.h
 * @brief Radix sort for fixed-width integer arrays
 */
#pragma once
#include <QObject>
#include <QVector>
#include <QByteArray>

/**
 * @brief Radix sort for fixed-width integer arrays
 */
class RadixSort6 : public QObject {
    Q_OBJECT
public:
    struct Stats {
        quint64 calls = 0;
        quint64 itemsProcessed = 0;
        quint64 errors = 0;
    };

    explicit RadixSort6(QObject *parent = nullptr) : QObject(parent) {}
    ~RadixSort6() override = default;

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

