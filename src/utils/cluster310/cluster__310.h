/**
 * @file cluster__310.h
 * @brief cluster module cluster__310
 */
#pragma once
#include <QObject>
#include <QVector>
class cluster__310 : public QObject {
    Q_OBJECT
public:
    struct Stats { quint64 calls=0; quint64 items=0; quint64 errors=0; };
    explicit cluster__310(QObject *p=nullptr) : QObject(p) {}
    ~cluster__310() override = default;
    QVector<double> compute(const QVector<double> &input);
    Stats stats() const { return m_stats; }
    void resetStats() { m_stats = {}; }
signals:
    void computed(const QVector<double> &result);
private:
    Stats m_stats;
};

