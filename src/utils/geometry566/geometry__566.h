/**
 * @file geometry__566.h
 * @brief geometry module geometry__566
 */
#pragma once
#include <QObject>
#include <QVector>
class geometry__566 : public QObject {
    Q_OBJECT
public:
    struct Stats { quint64 calls=0; quint64 items=0; quint64 errors=0; };
    explicit geometry__566(QObject *p=nullptr) : QObject(p) {}
    ~geometry__566() override = default;
    QVector<double> compute(const QVector<double> &input);
    Stats stats() const { return m_stats; }
    void resetStats() { m_stats = {}; }
signals:
    void computed(const QVector<double> &result);
private:
    Stats m_stats;
};

