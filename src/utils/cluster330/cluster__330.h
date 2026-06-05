/**
 * @file cluster__330.h
 * @brief cluster module cluster__330
 */
#pragma once
#include <QObject>
#include <QVector>
class cluster__330 : public QObject {
    Q_OBJECT
public:
    struct Stats { quint64 calls=0; quint64 items=0; quint64 errors=0; };
    explicit cluster__330(QObject *p=nullptr) : QObject(p) {}
    ~cluster__330() override = default;
    QVector<double> compute(const QVector<double> &input);
    Stats stats() const { return m_stats; }
    void resetStats() { m_stats = {}; }
signals:
    void computed(const QVector<double> &result);
private:
    Stats m_stats;
};

