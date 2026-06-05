/**
 * @file compress__432.h
 * @brief compress module compress__432
 */
#pragma once
#include <QObject>
#include <QVector>
class compress__432 : public QObject {
    Q_OBJECT
public:
    struct Stats { quint64 calls=0; quint64 items=0; quint64 errors=0; };
    explicit compress__432(QObject *p=nullptr) : QObject(p) {}
    ~compress__432() override = default;
    QVector<double> compute(const QVector<double> &input);
    Stats stats() const { return m_stats; }
    void resetStats() { m_stats = {}; }
signals:
    void computed(const QVector<double> &result);
private:
    Stats m_stats;
};

