/**
 * @file sort__670.h
 * @brief sort module sort__670
 */
#pragma once
#include <QObject>
#include <QVector>
class sort__670 : public QObject {
    Q_OBJECT
public:
    struct Stats { quint64 calls=0; quint64 items=0; quint64 errors=0; };
    explicit sort__670(QObject *p=nullptr) : QObject(p) {}
    ~sort__670() override = default;
    QVector<double> compute(const QVector<double> &input);
    Stats stats() const { return m_stats; }
    void resetStats() { m_stats = {}; }
signals:
    void computed(const QVector<double> &result);
private:
    Stats m_stats;
};

