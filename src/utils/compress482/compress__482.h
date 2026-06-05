/**
 * @file compress__482.h
 * @brief compress module compress__482
 */
#pragma once
#include <QObject>
#include <QVector>
class compress__482 : public QObject {
    Q_OBJECT
public:
    struct Stats { quint64 calls=0; quint64 items=0; quint64 errors=0; };
    explicit compress__482(QObject *p=nullptr) : QObject(p) {}
    ~compress__482() override = default;
    QVector<double> compute(const QVector<double> &input);
    Stats stats() const { return m_stats; }
    void resetStats() { m_stats = {}; }
signals:
    void computed(const QVector<double> &result);
private:
    Stats m_stats;
};

