/**
 * @file ChebyshevPoly3.h
 * @brief Chebyshev polynomial for filter design
 */
#pragma once
#include <QObject>
#include <QVector>
#include <QByteArray>

/**
 * @brief Chebyshev polynomial for filter design
 */
class ChebyshevPoly3 : public QObject {
    Q_OBJECT
public:
    struct Stats {
        quint64 calls = 0;
        quint64 itemsProcessed = 0;
        quint64 errors = 0;
    };

    explicit ChebyshevPoly3(QObject *parent = nullptr) : QObject(parent) {}
    ~ChebyshevPoly3() override = default;

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

