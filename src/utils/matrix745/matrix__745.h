/**
 * @file matrix__745.h
 * @brief matrix module matrix__745
 */
#pragma once
#include <QObject>
#include <QVector>
class matrix__745 : public QObject {
    Q_OBJECT
public:
    struct Stats { quint64 calls=0; quint64 items=0; quint64 errors=0; };
    explicit matrix__745(QObject *p=nullptr) : QObject(p) {}
    ~matrix__745() override = default;
    QVector<double> compute(const QVector<double> &input);
    Stats stats() const { return m_stats; }
    void resetStats() { m_stats = {}; }
signals:
    void computed(const QVector<double> &result);
private:
    Stats m_stats;
};

