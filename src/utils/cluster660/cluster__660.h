/**
 * @file cluster__660.h
 * @brief cluster module cluster__660
 */
#pragma once
#include <QObject>
#include <QVector>
class cluster__660 : public QObject {
    Q_OBJECT
public:
    struct Stats { quint64 calls=0; quint64 items=0; quint64 errors=0; };
    explicit cluster__660(QObject *p=nullptr) : QObject(p) {}
    ~cluster__660() override = default;
    QVector<double> compute(const QVector<double> &input);
    Stats stats() const { return m_stats; }
    void resetStats() { m_stats = {}; }
signals:
    void computed(const QVector<double> &result);
private:
    Stats m_stats;
};

