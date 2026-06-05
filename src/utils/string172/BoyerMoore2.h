/**
 * @file BoyerMoore2.h
 * @brief Boyer-Moore string search algorithm
 */
#pragma once
#include <QObject>
#include <QVector>
#include <QByteArray>

/**
 * @brief Boyer-Moore string search algorithm
 */
class BoyerMoore2 : public QObject {
    Q_OBJECT
public:
    struct Stats { quint64 calls = 0; quint64 items = 0; quint64 errors = 0; };
    explicit BoyerMoore2(QObject *p = nullptr) : QObject(p) {}
    ~BoyerMoore2() override = default;
    QVector<double> compute(const QVector<double> &input);
    Stats stats() const { return m_stats; }
    void resetStats() { m_stats = {}; }
signals:
    void computed(const QVector<double> &result);
private:
    Stats m_stats;
};

