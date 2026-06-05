/**
 * @file compress__662.h
 * @brief compress module compress__662
 */
#pragma once
#include <QObject>
#include <QVector>
class compress__662 : public QObject {
    Q_OBJECT
public:
    struct Stats { quint64 calls=0; quint64 items=0; quint64 errors=0; };
    explicit compress__662(QObject *p=nullptr) : QObject(p) {}
    ~compress__662() override = default;
    QVector<double> compute(const QVector<double> &input);
    Stats stats() const { return m_stats; }
    void resetStats() { m_stats = {}; }
signals:
    void computed(const QVector<double> &result);
private:
    Stats m_stats;
};

