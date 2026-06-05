/**
 * @file interp__421.h
 * @brief interp module interp__421
 */
#pragma once
#include <QObject>
#include <QVector>
class interp__421 : public QObject {
    Q_OBJECT
public:
    struct Stats { quint64 calls=0; quint64 items=0; quint64 errors=0; };
    explicit interp__421(QObject *p=nullptr) : QObject(p) {}
    ~interp__421() override = default;
    QVector<double> compute(const QVector<double> &input);
    Stats stats() const { return m_stats; }
    void resetStats() { m_stats = {}; }
signals:
    void computed(const QVector<double> &result);
private:
    Stats m_stats;
};

