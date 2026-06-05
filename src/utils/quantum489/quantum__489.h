/**
 * @file quantum__489.h
 * @brief quantum module quantum__489
 */
#pragma once
#include <QObject>
#include <QVector>
class quantum__489 : public QObject {
    Q_OBJECT
public:
    struct Stats { quint64 calls=0; quint64 items=0; quint64 errors=0; };
    explicit quantum__489(QObject *p=nullptr) : QObject(p) {}
    ~quantum__489() override = default;
    QVector<double> compute(const QVector<double> &input);
    Stats stats() const { return m_stats; }
    void resetStats() { m_stats = {}; }
signals:
    void computed(const QVector<double> &result);
private:
    Stats m_stats;
};

