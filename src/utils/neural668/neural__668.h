/**
 * @file neural__668.h
 * @brief neural module neural__668
 */
#pragma once
#include <QObject>
#include <QVector>
class neural__668 : public QObject {
    Q_OBJECT
public:
    struct Stats { quint64 calls=0; quint64 items=0; quint64 errors=0; };
    explicit neural__668(QObject *p=nullptr) : QObject(p) {}
    ~neural__668() override = default;
    QVector<double> compute(const QVector<double> &input);
    Stats stats() const { return m_stats; }
    void resetStats() { m_stats = {}; }
signals:
    void computed(const QVector<double> &result);
private:
    Stats m_stats;
};

