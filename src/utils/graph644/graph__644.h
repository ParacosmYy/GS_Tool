/**
 * @file graph__644.h
 * @brief graph module graph__644
 */
#pragma once
#include <QObject>
#include <QVector>
class graph__644 : public QObject {
    Q_OBJECT
public:
    struct Stats { quint64 calls=0; quint64 items=0; quint64 errors=0; };
    explicit graph__644(QObject *p=nullptr) : QObject(p) {}
    ~graph__644() override = default;
    QVector<double> compute(const QVector<double> &input);
    Stats stats() const { return m_stats; }
    void resetStats() { m_stats = {}; }
signals:
    void computed(const QVector<double> &result);
private:
    Stats m_stats;
};

