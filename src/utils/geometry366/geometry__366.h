/**
 * @file geometry__366.h
 * @brief geometry module geometry__366
 */
#pragma once
#include <QObject>
#include <QVector>
class geometry__366 : public QObject {
    Q_OBJECT
public:
    struct Stats { quint64 calls=0; quint64 items=0; quint64 errors=0; };
    explicit geometry__366(QObject *p=nullptr) : QObject(p) {}
    ~geometry__366() override = default;
    QVector<double> compute(const QVector<double> &input);
    Stats stats() const { return m_stats; }
    void resetStats() { m_stats = {}; }
signals:
    void computed(const QVector<double> &result);
private:
    Stats m_stats;
};

