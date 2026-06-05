/**
 * @file interp__351.h
 * @brief interp module interp__351
 */
#pragma once
#include <QObject>
#include <QVector>
class interp__351 : public QObject {
    Q_OBJECT
public:
    struct Stats { quint64 calls=0; quint64 items=0; quint64 errors=0; };
    explicit interp__351(QObject *p=nullptr) : QObject(p) {}
    ~interp__351() override = default;
    QVector<double> compute(const QVector<double> &input);
    Stats stats() const { return m_stats; }
    void resetStats() { m_stats = {}; }
signals:
    void computed(const QVector<double> &result);
private:
    Stats m_stats;
};

