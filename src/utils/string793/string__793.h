/**
 * @file string__793.h
 * @brief string module string__793
 */
#pragma once
#include <QObject>
#include <QVector>
class string__793 : public QObject {
    Q_OBJECT
public:
    struct Stats { quint64 calls=0; quint64 items=0; quint64 errors=0; };
    explicit string__793(QObject *p=nullptr) : QObject(p) {}
    ~string__793() override = default;
    QVector<double> compute(const QVector<double> &input);
    Stats stats() const { return m_stats; }
    void resetStats() { m_stats = {}; }
signals:
    void computed(const QVector<double> &result);
private:
    Stats m_stats;
};

