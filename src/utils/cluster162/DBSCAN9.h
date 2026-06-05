/**
 * @file DBSCAN9.h
 * @brief Density-based spatial clustering with noise
 */
#pragma once
#include <QObject>
#include <QVector>
#include <QByteArray>

/**
 * @brief Density-based spatial clustering with noise
 */
class DBSCAN9 : public QObject {
    Q_OBJECT
public:
    struct Stats { quint64 calls = 0; quint64 items = 0; quint64 errors = 0; };
    explicit DBSCAN9(QObject *p = nullptr) : QObject(p) {}
    ~DBSCAN9() override = default;
    QVector<double> compute(const QVector<double> &input);
    Stats stats() const { return m_stats; }
    void resetStats() { m_stats = {}; }
signals:
    void computed(const QVector<double> &result);
private:
    Stats m_stats;
};

