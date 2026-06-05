/**
 * @file matrix__355.h
 * @brief matrix module matrix__355
 */
#pragma once
#include <QObject>
#include <QVector>
class matrix__355 : public QObject {
    Q_OBJECT
public:
    struct Stats { quint64 calls=0; quint64 items=0; quint64 errors=0; };
    explicit matrix__355(QObject *p=nullptr) : QObject(p) {}
    ~matrix__355() override = default;
    QVector<double> compute(const QVector<double> &input);
    Stats stats() const { return m_stats; }
    void resetStats() { m_stats = {}; }
signals:
    void computed(const QVector<double> &result);
private:
    Stats m_stats;
};

