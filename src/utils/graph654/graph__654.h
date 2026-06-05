/**
 * @file graph__654.h
 * @brief graph module graph__654
 */
#pragma once
#include <QObject>
#include <QVector>
class graph__654 : public QObject {
    Q_OBJECT
public:
    struct Stats { quint64 calls=0; quint64 items=0; quint64 errors=0; };
    explicit graph__654(QObject *p=nullptr) : QObject(p) {}
    ~graph__654() override = default;
    QVector<double> compute(const QVector<double> &input);
    Stats stats() const { return m_stats; }
    void resetStats() { m_stats = {}; }
signals:
    void computed(const QVector<double> &result);
private:
    Stats m_stats;
};

