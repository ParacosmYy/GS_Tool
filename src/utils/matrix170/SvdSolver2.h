/**
 * @file SvdSolver2.h
 * @brief Singular value decomposition
 */
#pragma once
#include <QObject>
#include <QVector>
#include <QByteArray>

/**
 * @brief Singular value decomposition
 */
class SvdSolver2 : public QObject {
    Q_OBJECT
public:
    struct Stats { quint64 calls = 0; quint64 items = 0; quint64 errors = 0; };
    explicit SvdSolver2(QObject *p = nullptr) : QObject(p) {}
    ~SvdSolver2() override = default;
    QVector<double> compute(const QVector<double> &input);
    Stats stats() const { return m_stats; }
    void resetStats() { m_stats = {}; }
signals:
    void computed(const QVector<double> &result);
private:
    Stats m_stats;
};

