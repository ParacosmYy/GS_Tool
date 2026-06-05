/**
 * @file LZ77Compress2.h
 * @brief optim algorithm module - LZ77Compress2
 */
#pragma once
#include <QObject>
#include <QVector>
class LZ77Compress2 : public QObject {
    Q_OBJECT
public:
    struct Stats { quint64 calls = 0; quint64 items = 0; quint64 errors = 0; };
    explicit LZ77Compress2(QObject *p = nullptr) : QObject(p) {}
    ~LZ77Compress2() override = default;
    QVector<double> compute(const QVector<double> &input);
    Stats stats() const { return m_stats; }
    void resetStats() { m_stats = {}; }
signals:
    void computed(const QVector<double> &result);
private:
    Stats m_stats;
};

