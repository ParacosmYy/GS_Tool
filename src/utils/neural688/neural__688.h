/**
 * @file neural__688.h
 * @brief neural module neural__688
 */
#pragma once
#include <QObject>
#include <QVector>
class neural__688 : public QObject {
    Q_OBJECT
public:
    struct Stats { quint64 calls=0; quint64 items=0; quint64 errors=0; };
    explicit neural__688(QObject *p=nullptr) : QObject(p) {}
    ~neural__688() override = default;
    QVector<double> compute(const QVector<double> &input);
    Stats stats() const { return m_stats; }
    void resetStats() { m_stats = {}; }
signals:
    void computed(const QVector<double> &result);
private:
    Stats m_stats;
};

