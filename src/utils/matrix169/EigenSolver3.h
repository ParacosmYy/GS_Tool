/**
 * @file EigenSolver3.h
 * @brief Eigenvalue decomposition solver
 */
#pragma once
#include <QObject>
#include <QVector>
#include <QByteArray>

/**
 * @brief Eigenvalue decomposition solver
 */
class EigenSolver3 : public QObject {
    Q_OBJECT
public:
    struct Stats { quint64 calls = 0; quint64 items = 0; quint64 errors = 0; };
    explicit EigenSolver3(QObject *p = nullptr) : QObject(p) {}
    ~EigenSolver3() override = default;
    QVector<double> compute(const QVector<double> &input);
    Stats stats() const { return m_stats; }
    void resetStats() { m_stats = {}; }
signals:
    void computed(const QVector<double> &result);
private:
    Stats m_stats;
};

