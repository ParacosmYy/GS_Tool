/**
 * @file cluster__410.h
 * @brief cluster module cluster__410
 */
#pragma once
#include <QObject>
#include <QVector>
class cluster__410 : public QObject {
    Q_OBJECT
public:
    struct Stats { quint64 calls=0; quint64 items=0; quint64 errors=0; };
    explicit cluster__410(QObject *p=nullptr) : QObject(p) {}
    ~cluster__410() override = default;
    QVector<double> compute(const QVector<double> &input);
    Stats stats() const { return m_stats; }
    void resetStats() { m_stats = {}; }
signals:
    void computed(const QVector<double> &result);
private:
    Stats m_stats;
};

