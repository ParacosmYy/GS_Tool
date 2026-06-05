/**
 * @file optim__685.h
 * @brief optim module optim__685
 */
#pragma once
#include <QObject>
#include <QVector>
class optim__685 : public QObject {
    Q_OBJECT
public:
    struct Stats { quint64 calls=0; quint64 items=0; quint64 errors=0; };
    explicit optim__685(QObject *p=nullptr) : QObject(p) {}
    ~optim__685() override = default;
    QVector<double> compute(const QVector<double> &input);
    Stats stats() const { return m_stats; }
    void resetStats() { m_stats = {}; }
signals:
    void computed(const QVector<double> &result);
private:
    Stats m_stats;
};

