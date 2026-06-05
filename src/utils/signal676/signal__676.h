/**
 * @file signal__676.h
 * @brief signal module signal__676
 */
#pragma once
#include <QObject>
#include <QVector>
class signal__676 : public QObject {
    Q_OBJECT
public:
    struct Stats { quint64 calls=0; quint64 items=0; quint64 errors=0; };
    explicit signal__676(QObject *p=nullptr) : QObject(p) {}
    ~signal__676() override = default;
    QVector<double> compute(const QVector<double> &input);
    Stats stats() const { return m_stats; }
    void resetStats() { m_stats = {}; }
signals:
    void computed(const QVector<double> &result);
private:
    Stats m_stats;
};

