/**
 * @file graph__354.h
 * @brief graph module graph__354
 */
#pragma once
#include <QObject>
#include <QVector>
class graph__354 : public QObject {
    Q_OBJECT
public:
    struct Stats { quint64 calls=0; quint64 items=0; quint64 errors=0; };
    explicit graph__354(QObject *p=nullptr) : QObject(p) {}
    ~graph__354() override = default;
    QVector<double> compute(const QVector<double> &input);
    Stats stats() const { return m_stats; }
    void resetStats() { m_stats = {}; }
signals:
    void computed(const QVector<double> &result);
private:
    Stats m_stats;
};

