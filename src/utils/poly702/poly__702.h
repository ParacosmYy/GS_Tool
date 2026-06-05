/**
 * @file poly__702.h
 * @brief poly module poly__702
 */
#pragma once
#include <QObject>
#include <QVector>
class poly__702 : public QObject {
    Q_OBJECT
public:
    struct Stats { quint64 calls=0; quint64 items=0; quint64 errors=0; };
    explicit poly__702(QObject *p=nullptr) : QObject(p) {}
    ~poly__702() override = default;
    QVector<double> compute(const QVector<double> &input);
    Stats stats() const { return m_stats; }
    void resetStats() { m_stats = {}; }
signals:
    void computed(const QVector<double> &result);
private:
    Stats m_stats;
};

